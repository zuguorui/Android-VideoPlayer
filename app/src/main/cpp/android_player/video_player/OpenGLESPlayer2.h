//
// Created by incus on 2020-04-21.
//

#ifndef ANDROID_OPENGLTEST2_OPENGLESPLAYER2_H
#define ANDROID_OPENGLTEST2_OPENGLESPLAYER2_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <thread>
#include <list>

#include "Texture.h"
#include "Render.h"
#include "EGLCore.h"
#include "IVideoPlayer.h"
#include "IVideoFrameProvider.h"

class OpenGLESPlayer2: public IVideoPlayer {
public:
    OpenGLESPlayer2();
    ~OpenGLESPlayer2();

    bool create() override;

    void release() override;

    void refresh() override;

    void setVideoFrameProvider(IVideoFrameProvider *provider) override;

    void removeVideoFrameProvider(IVideoFrameProvider *provider) override;

    void setWindow(void *window) override;

    void setSize(int32_t width, int32_t height) override;

    bool isReady() override;

private:
    EGLCore *eglCore = NULL;
    Texture *texture = NULL;
    Render *render = NULL;
    EGLSurface surface = EGL_NO_SURFACE;

    ANativeWindow *window = NULL;

    int32_t width = 0;
    int32_t height = 0;

    enum RenderMessage{
        SET_WINDOW, REFRESH, SET_SIZE, EXIT
    };

    list<RenderMessage> messageQueue;
    mutex renderMu;
    condition_variable newMessageSignal;
    thread *renderThread = NULL;

    IVideoFrameProvider *frameProvider = NULL;


    bool initComponents();
    void releaseComponents();

    void updateTexImage();
    void drawFrame();

    static void threadCallback(void *self);
    void renderLoop();


};


#endif //ANDROID_OPENGLTEST2_OPENGLESPLAYER2_H
