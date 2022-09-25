//
// Created by incus on 2020-04-15.
//

#ifndef ANDROID_VIDEOPLAYER_OPENGLESPLAYER_H
#define ANDROID_VIDEOPLAYER_OPENGLESPLAYER_H

#include <iostream>
#include <stdlib.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <thread>

#include "Texture.h"
#include "Render.h"
#include "EGLCore.h"
#include "BlockRecyclerQueue.h"

#include "IVideoPlayer.h"

class OpenGLESPlayer: public IVideoPlayer {
public:
    OpenGLESPlayer();

    ~OpenGLESPlayer();

    bool create() override;

    void release() override;


    void setVideoFrameProvider(IVideoFrameProvider *provider) override;

    void removeVideoFrameProvider(IVideoFrameProvider *provider) override;

    void setWindow(void *window) override;

    void setSize(int32_t width, int32_t height) override;

    void refresh() override;

    bool isReady() override;

private:

    void renderLoop();

    bool initComponents();

    void releaseComponents();

    void updateTexImage();

    void drawFrame();

    static void threadCallback(void *self);

    Texture *mTexture;
    Render *mRender;
    EGLCore *eglCore;
    EGLSurface mSurface;

    int width;
    int height;

    thread *renderThread = NULL;

    ANativeWindow *window = NULL;
    mutex windowMu;
    condition_variable setWindowSignal;


    IVideoFrameProvider *provider = NULL;
    mutex providerMu;

    bool setSizeFlag = false;
    bool exitFlag = false;

};


#endif //ANDROID_VIDEOPLAYER_OPENGLESPLAYER_H
