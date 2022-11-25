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

#include "IVideoOutput.h"
#include "PlayerContext.h"
#include "LinkedBlockingQueue.h"

class OpenGLESPlayer2: public IVideoOutput {
public:
    OpenGLESPlayer2(PlayerContext *playerContext);
    ~OpenGLESPlayer2();

    bool create() override;

    void release() override;

    void setWindow(void *window) override;

    void setSize(int32_t width, int32_t height) override;

    bool isReady() override;

    void write(VideoFrame* frame) override;

private:
    EGLCore *eglCore = nullptr;
    Texture *texture = nullptr;
    Render *render = nullptr;
    EGLSurface surface = EGL_NO_SURFACE;

    ANativeWindow *window = nullptr;

    int32_t width = 0;
    int32_t height = 0;

    enum RenderMessage{
        SET_WINDOW, REFRESH, SET_SIZE, EXIT
    };

    std::thread *renderThread = nullptr;

    LinkedBlockingQueue<RenderMessage> messageQueue = LinkedBlockingQueue<RenderMessage>(10);
    LinkedBlockingQueue<VideoFrame *> frameQueue = LinkedBlockingQueue<VideoFrame *>(10);

    bool initComponents();
    void releaseComponents();

    void updateTexImage();
    void drawFrame();

    static void renderCallback(void *self);
    void renderLoop();


};


#endif //ANDROID_OPENGLTEST2_OPENGLESPLAYER2_H
