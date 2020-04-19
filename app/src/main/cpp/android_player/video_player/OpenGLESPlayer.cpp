//
// Created by incus on 2020-04-15.
//

#include "OpenGLESPlayer.h"
#include <android/log.h>

#define MODULE_NAME "OpenGLESPlayer"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

OpenGLESPlayer::OpenGLESPlayer() {
    LOGD("OpenGLESPlayer init");
    surface = EGL_NO_SURFACE;
    width = 1920;
    height = 1080;

    exitFlag = false;
    setSizeFlag = false;
}

OpenGLESPlayer::~OpenGLESPlayer() {
    LOGD("OpenGLESPlayer delete");

}

void* OpenGLESPlayer::threadCallback(void *self) {
    OpenGLESPlayer *player = (OpenGLESPlayer *)self;
    player->renderLoop();
}

bool OpenGLESPlayer::initComponents() {
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

    updateTexImage();

    render = new Render();
    if(!render->init(width, height, texture))
    {
        LOGE("init render failed");
        releaseComponents();
        return false;
    }
    return true;
}

void OpenGLESPlayer::releaseComponents() {
    if(eglCore)
    {
        eglCore->releaseSurface(surface);
        eglCore->release();
        delete(eglCore);
        eglCore = NULL;
    }

    if(texture)
    {
        delete(texture);
    }
    if(render)
    {
        render->dealloc();
        delete(render);
    }
}

void OpenGLESPlayer::drawFrame() {
    LOGD("drawFrame");
    render->render();
    if(!eglCore->swapBuffers(render))
    {
        LOGE("swap buffers failed");
    }
}

void OpenGLESPlayer::updateTexImage() {
    LOGD("updateTexImage");
    unique_lock<mutex> providerLock(providerMu);
    if(provider == NULL)
    {
        return;
    }
    VideoFrame *frame = provider->getVideoFrame();
    LOGD("video frame width = %d, height = %d, pts = %ld", frame->width, frame->height, frame->pts);
    texture->updateDataToTexture(frame->data, frame->width, frame->height);
    provider->putBackUsed(frame);
    providerLock.unlock();
}

void OpenGLESPlayer::renderLoop() {
    LOGD("render loop enter");
    unique_lock<mutex> windowLock(windowMu);
    while(window == NULL)
    {
        setWindowSignal.wait(windowLock);
        if(exitFlag)
        {
            windowLock.unlock();
            return;
        }
    }
    if(!initComponents())
    {
        LOGE("init components failed");
        windowLock.unlock();
        return;
    }
    windowLock.unlock();

    while(!exitFlag)
    {
        if(setSizeFlag)
        {
            setSizeFlag = false;
            render->resetRenderSize(0, 0, width, height);
        }
        updateTexImage();
        eglCore->makeCurrent(surface);
        drawFrame();

    }
}

bool OpenGLESPlayer::create() {
    renderThread = new thread(threadCallback, this);
    LOGD("create");
    return true;
}

void OpenGLESPlayer::release() {
    exitFlag = true;
    setWindowSignal.notify_all();
    if(renderThread != NULL && renderThread->joinable())
    {
        renderThread->join();
    }

}

void OpenGLESPlayer::start() {
    LOGD("gl player start called");
}

void OpenGLESPlayer::stop() {
    LOGD("gl player stop called");
}

void OpenGLESPlayer::setVideoFrameProvider(IVideoFrameProvider *provider) {
    unique_lock<mutex> providerLock(providerMu);
    this->provider = provider;
    providerLock.unlock();
}

void OpenGLESPlayer::removeVideoFrameProvider(IVideoFrameProvider *provider) {
    if(this->provider == provider)
    {
        unique_lock<mutex> providerLock(providerMu);
        this->provider = NULL;
        providerLock.unlock();
    }

}

void OpenGLESPlayer::setWindow(void *window) {
    LOGD("setWindow");
    unique_lock<mutex> windowLock(windowMu);
    this->window = (ANativeWindow *)window;
    windowLock.unlock();
    setWindowSignal.notify_all();
}

void OpenGLESPlayer::setSize(int32_t width, int32_t height) {
    this->width = width;
    this->height = height;
    setSizeFlag = true;
}
