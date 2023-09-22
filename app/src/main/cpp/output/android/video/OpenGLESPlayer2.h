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

#include "GLESRender.h"

#include "IVideoOutput.h"
#include "PlayerContext.h"
#include "LinkedBlockingQueue.h"

class OpenGLESPlayer2: public IVideoOutput {
public:
    OpenGLESPlayer2(PlayerContext *playerContext);
    ~OpenGLESPlayer2();

    bool setFormat(AVPixelFormat pixelFormat, AVColorSpace colorSpace, bool isHDR) override;

    bool create(void *surface) override;

    void release() override;

    void setScreenSize(int32_t width, int32_t height) override;

    bool isReady() override;

    void write(VideoFrame* frame) override;

    void setSizeMode(SizeMode mode) override;

private:

    GLESRender render;

    ANativeWindow *window = nullptr;

    int32_t screenWidth = 0;
    int32_t screenHeight = 0;

    AVPixelFormat format = AVPixelFormat::AV_PIX_FMT_NONE;
    AVColorSpace colorSpace;
    bool isHDR = false;

    SizeMode sizeMode;

    enum RenderMessage{
        SET_WINDOW,
        REFRESH,
        SET_SCREEN_SIZE,
        SET_SRC_FORMAT,
        EXIT,
        SET_SIZE_MODE,
    };

    std::thread *renderThread = nullptr;

    LinkedBlockingQueue<RenderMessage> messageQueue = LinkedBlockingQueue<RenderMessage>(10);
    LinkedBlockingQueue<VideoFrame *> frameQueue = LinkedBlockingQueue<VideoFrame *>(10);

    static void renderCallback(void *self);
    void renderLoop();


};


#endif //ANDROID_OPENGLTEST2_OPENGLESPLAYER2_H
