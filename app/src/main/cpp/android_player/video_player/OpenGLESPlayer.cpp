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
    mSurface = EGL_NO_SURFACE;
    width = 1920;
    height = 1080;

    exitFlag = false;
    setSizeFlag = false;
}

OpenGLESPlayer::~OpenGLESPlayer() {
    LOGD("OpenGLESPlayer delete");

}

void OpenGLESPlayer::threadCallback(void *self) {
    OpenGLESPlayer *player = (OpenGLESPlayer *)self;
    player->renderLoop();
}

bool OpenGLESPlayer::initComponents() {
    LOGD("init");
    eglCore = new EGLCore();
    eglCore->init();
    mSurface = eglCore->createWindowSurface(window);
    eglCore->makeCurrent(mSurface);

    mTexture = new Texture();

    if(!(mTexture->createTexture()))
    {
        LOGE("create texture failed");
        releaseComponents();
        return false;
    }

//    updateTexImage();

    mRender = new Render();
    if(!mRender->init(width, height, mTexture))
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
        eglCore->releaseSurface(mSurface);
        eglCore->release();
        delete(eglCore);
        eglCore = NULL;
    }

    if(mTexture)
    {
        delete(mTexture);
    }
    if(mRender)
    {
        mRender->dealloc();
        delete(mRender);
    }
}

void OpenGLESPlayer::drawFrame() {
    LOGD("drawFrame");
    mRender->render();
    if(!eglCore->swapBuffers(mRender))
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
    uint8_t *pixel = (uint8_t *)malloc(frame->width * frame->height * 3 * sizeof(uint8_t));
    for(int i = 0; i < frame->width * frame->height; i++)
    {

        pixel[3 * i] = 0xff;
        pixel[3 * i + 1] = 0x00;
        pixel[3 * i + 2] = 0x00;

    }
    mTexture->updateDataToTexture(pixel, frame->width, frame->height);
    free(pixel);
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
            mRender->resetRenderSize(0, 0, width, height);
        }
        if(eglCore)
        {
            updateTexImage();
            eglCore->makeCurrent(mSurface);
            drawFrame();
        }


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
    setWindowSignal.notify_all();
    windowLock.unlock();

}

void OpenGLESPlayer::setSize(int32_t width, int32_t height) {
    this->width = width;
    this->height = height;
    setSizeFlag = true;
}

void OpenGLESPlayer::refresh() {

}

bool OpenGLESPlayer::isReady() {
    return false;
}
