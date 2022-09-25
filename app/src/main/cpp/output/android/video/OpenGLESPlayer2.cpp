//
// Created by incus on 2020-04-21.
//

#include "OpenGLESPlayer2.h"

#include "Log.h"

#define TAG "OpenGLESPlayer2"

using namespace std;

OpenGLESPlayer2::OpenGLESPlayer2(PlayerContext *playerContext): IVideoOutput(playerContext) {

}

OpenGLESPlayer2::~OpenGLESPlayer2() {

}

bool OpenGLESPlayer2::create() {
    renderThread = new thread(renderCallback, this);
    return true;
}

void OpenGLESPlayer2::release() {
    messageQueue.setBlockingPop(false);
    messageQueue.setBlockingPush(false);
    messageQueue.push(RenderMessage::EXIT);
    if(renderThread != nullptr && renderThread->joinable())
    {
        renderThread->join();
    }

    if(window != nullptr)
    {
        ANativeWindow_release(window);
        window = nullptr;
    }
}


void OpenGLESPlayer2::renderCallback(void *self) {
    ((OpenGLESPlayer2 *)self)->renderLoop();
}


void OpenGLESPlayer2::setWindow(void *window) {
    this->window = (ANativeWindow *)window;
    messageQueue.push(RenderMessage::SET_WINDOW);
}

void OpenGLESPlayer2::setSize(int32_t width, int32_t height) {

    this->width = width;
    this->height = height;
    messageQueue.push(RenderMessage::SET_SIZE);
}

void OpenGLESPlayer2::renderLoop() {
    LOGD(TAG, "renderLoop start");
    optional<RenderMessage> messageOpt;
    RenderMessage message;
    bool exitFlag = false;

    while(!exitFlag)
    {

        messageOpt = messageQueue.pop();
        if (!messageOpt.has_value()) {
            LOGE(TAG, "messageOpt is null");
            continue;
        }
        message = messageOpt.value();

        switch (message)
        {
            case SET_WINDOW:
                initComponents();
                break;
            case SET_SIZE:
                render->resetRenderSize(0, 0, width, height);
                break;
            case REFRESH:
                if(eglCore && render && texture)
                {
                    updateTexImage();
                    eglCore->makeCurrent(surface);
                    drawFrame();
                }
                break;
            case EXIT:
                exitFlag = true;
                break;
            default:
                break;
        }

    }

    releaseComponents();
}

bool OpenGLESPlayer2::initComponents() {
    LOGD(TAG, "init");
    eglCore = new EGLCore();
    eglCore->init();
    surface = eglCore->createWindowSurface(window);
    eglCore->makeCurrent(surface);

    texture = new Texture();

    if(!(texture->createTexture()))
    {
        LOGE(TAG, "create texture failed");
        releaseComponents();
        return false;
    }


    render = new Render();
    if(!render->init(width, height, texture))
    {
        LOGE(TAG, "init render failed");
        releaseComponents();
        return false;
    }
    return true;
}

void OpenGLESPlayer2::releaseComponents() {
    if(eglCore)
    {
        eglCore->releaseSurface(surface);
        eglCore->release();
        delete(eglCore);
        eglCore = nullptr;
    }

    if(texture)
    {
        texture->dealloc();
        delete(texture);
        texture = nullptr;
    }
    if(render)
    {
        render->dealloc();
        delete(render);
        render = nullptr;
    }

    window = nullptr;

    optional<unique_ptr<VideoFrame>> frameOpt;
    while (frameQueue.getSize() > 0) {
        frameOpt = frameQueue.pop(false);
        if (!frameOpt.has_value()) {
            break;
        }

        if (frameOpt.value() != nullptr) {
            frameOpt.value().reset();
        }
    }

    frameQueue.setBlockingPush(true);
    frameQueue.setBlockingPop(true);
}

void OpenGLESPlayer2::updateTexImage() {
    optional<unique_ptr<VideoFrame>> frameOpt = frameQueue.pop();
    if (!frameOpt.has_value()) {
        LOGE(TAG, "frameOpt has no value");
        return;
    }

    unique_ptr<VideoFrame> frame = std::move(frameOpt.value());
    if(frame == nullptr)
    {
        LOGE(TAG, "frame is null");
        return;
    }
    texture->updateDataToTexture(frame->data, frame->width, frame->height);
    if (playerCtx != nullptr) {
        playerCtx->recycleVideoFrame(frame);
    } else {
        frame.reset();
    }
}

void OpenGLESPlayer2::drawFrame() {
    LOGD(TAG, "drawFrame");
    render->render();
    if(!eglCore->swapBuffers(surface))
    {
        LOGE(TAG, "swap buffers failed");
    }
}

bool OpenGLESPlayer2::isReady() {
    return eglCore && texture && render;
}

void OpenGLESPlayer2::write(std::unique_ptr<VideoFrame> frame) {
    frameQueue.push(frame);
    messageQueue.push(RenderMessage::REFRESH);
}
