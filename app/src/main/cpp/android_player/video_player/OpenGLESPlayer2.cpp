//
// Created by incus on 2020-04-21.
//

#include "OpenGLESPlayer2.h"

#include <android/log.h>

#define MODULE_NAME "OpenGLESPlayer2"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

OpenGLESPlayer2::OpenGLESPlayer2() {

}

OpenGLESPlayer2::~OpenGLESPlayer2() {

}

bool OpenGLESPlayer2::create() {
    renderThread = new thread(threadCallback, this);
    return true;
}

void OpenGLESPlayer2::release() {
    unique_lock<mutex> locker(renderMu);
    messageQueue.push_back(RenderMessage::EXIT);
    newMessageSignal.notify_all();
    locker.unlock();
    if(renderThread != NULL && renderThread->joinable())
    {
        renderThread->join();
    }

    if(window != NULL)
    {
        ANativeWindow_release(window);
        window = NULL;
    }
}

void OpenGLESPlayer2::refresh() {
    unique_lock<mutex> locker(renderMu);
    messageQueue.push_back(RenderMessage::REFRESH);
    newMessageSignal.notify_all();
    locker.unlock();
}

void OpenGLESPlayer2::threadCallback(void *self) {
    ((OpenGLESPlayer2 *)self)->renderLoop();
}

void OpenGLESPlayer2::setVideoFrameProvider(IVideoFrameProvider *provider) {
    unique_lock<mutex> locker(renderMu);
    this->frameProvider = provider;
    locker.unlock();
}

void OpenGLESPlayer2::removeVideoFrameProvider(IVideoFrameProvider *provider) {
    unique_lock<mutex> locker(renderMu);
    this->frameProvider = NULL;
    locker.unlock();
}

void OpenGLESPlayer2::setWindow(void *window) {
    unique_lock<mutex> locker(renderMu);
    this->window = (ANativeWindow *)window;
    messageQueue.push_back(RenderMessage::SET_WINDOW);
    newMessageSignal.notify_all();
    locker.unlock();
}

void OpenGLESPlayer2::setSize(int32_t width, int32_t height) {
    unique_lock<mutex> locker(renderMu);
    this->width = width;
    this->height = height;
    messageQueue.push_back(RenderMessage::SET_SIZE);
    newMessageSignal.notify_all();
    locker.unlock();
}

void OpenGLESPlayer2::renderLoop() {
    LOGD("renderLoop start");
    RenderMessage message;
    bool exitFlag = false;

    while(!exitFlag)
    {
        unique_lock<mutex> locker(renderMu);
        while(messageQueue.size() == 0)
        {
            newMessageSignal.wait(locker);
        }
        message = messageQueue.front();
        messageQueue.pop_front();
        locker.unlock();
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
    LOGD("init");
    eglCore = new EGLCore();
    eglCore->init();
    surface = eglCore->createWindowSurface(window);
    eglCore->makeCurrent(surface);

    texture = new Texture();

    if(!(texture->createTexture()))
    {
        LOGE("create texture failed");
        releaseComponents();
        return false;
    }


    render = new Render();
    if(!render->init(width, height, texture))
    {
        LOGE("init render failed");
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
        eglCore = NULL;
    }

    if(texture)
    {
        texture->dealloc();
        delete(texture);
        texture = NULL;
    }
    if(render)
    {
        render->dealloc();
        delete(render);
        render = NULL;
    }

    window = NULL;
}

void OpenGLESPlayer2::updateTexImage() {
    if(frameProvider == NULL)
    {
        return;
    }
    VideoFrame *f = frameProvider->getVideoFrame();
    if(f == NULL)
    {
        return;
    }
    texture->updateDataToTexture(f->data, f->width, f->height);
    frameProvider->putBackUsed(f);
}

void OpenGLESPlayer2::drawFrame() {
    LOGD("drawFrame");
    render->render();
    if(!eglCore->swapBuffers(surface))
    {
        LOGE("swap buffers failed");
    }
}

bool OpenGLESPlayer2::isReady() {
    return eglCore && texture && render;
}
