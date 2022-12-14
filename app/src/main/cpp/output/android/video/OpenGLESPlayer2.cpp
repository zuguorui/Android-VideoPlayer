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
    LOGD(TAG, "create");
    renderThread = new thread(renderCallback, this);
    return true;
}

void OpenGLESPlayer2::release() {
    messageQueue.push(RenderMessage::EXIT);
    if(renderThread != nullptr && renderThread->joinable())
    {
        renderThread->join();
    }

//    if(window != nullptr)
//    {
//        ANativeWindow_release(window);
//        window = nullptr;
//    }
}


void OpenGLESPlayer2::renderCallback(void *self) {
    ((OpenGLESPlayer2 *)self)->renderLoop();
}


void OpenGLESPlayer2::setWindow(void *window) {
    LOGD(TAG, "setWindow");
    this->window = (ANativeWindow *)window;
    messageQueue.push(RenderMessage::SET_WINDOW);
}

void OpenGLESPlayer2::setScreenSize(int32_t width, int32_t height) {
    LOGD(TAG, "setScreenSize");
    this->screenWidth = width;
    this->screenHeight = height;
    messageQueue.push(RenderMessage::SET_SCREEN_SIZE);
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
        LOGD(TAG, "messsage = %d", message);
        switch (message)
        {
            case SET_WINDOW:
                if (!render.setWindow(window)) {
                    LOGE(TAG, "render setWindow failed");
                }
                break;
            case SET_SCREEN_SIZE:
                render.setScreenSize(screenWidth, screenHeight);
                break;
            case REFRESH:
            {
                optional<VideoFrame *> frameOpt = frameQueue.pop();
                if (frameOpt.has_value()) {
                    VideoFrame *frame = frameOpt.value();
                    if (render.isReady()) {
                        render.refresh(frame);
                    } else {
                        LOGE(TAG, "render is not ready");
                    }
                    playerCtx->recycleVideoFrame(frame);
                }
                break;
            }
            case SET_SRC_FORMAT:
                if (!render.create(format, colorSpace, isHDR)) {
                    LOGE(TAG, "render.create failed");
                }
                break;
            case EXIT:
                exitFlag = true;
                break;
            case SET_SIZE_MODE:
                render.setSizeMode(sizeMode);
                break;
            default:
                break;
        }

    }
    render.release();
}



void OpenGLESPlayer2::write(VideoFrame* frame) {
    LOGD(TAG, "write, pts = %lld, frameQueue.size = %ld", frame->pts, frameQueue.getSize());
    frameQueue.push(frame);
    messageQueue.push(RenderMessage::REFRESH);
}

void OpenGLESPlayer2::setSrcFormat(AVPixelFormat pixelFormat, AVColorSpace colorSpace, bool isHDR) {
    LOGD(TAG, "setSrcFormat");
    this->format = pixelFormat;
    this->colorSpace = colorSpace;
    this->isHDR = isHDR;
    messageQueue.push(RenderMessage::SET_SRC_FORMAT);
}

bool OpenGLESPlayer2::isReady() {
    return render.isReady();
}

void OpenGLESPlayer2::setSizeMode(SizeMode mode) {
    sizeMode = mode;
    messageQueue.push(RenderMessage::SET_SIZE_MODE);
}
