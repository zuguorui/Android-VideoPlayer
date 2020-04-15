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

#include "IVideoPlayer.h"

class OpenGLESPlayer: public IVideoPlayer {
public:
    OpenGLESPlayer();

    ~OpenGLESPlayer();

    bool create() override;

    void release() override;

    void start() override;

    void stop() override;

    void setVideoFrameProvider(IVideoFrameProvider *provider) override;

    void removeVideoFrameProvider(IVideoFrameProvider *provider) override;

    void setWindow(void *window) override;
};


#endif //ANDROID_VIDEOPLAYER_OPENGLESPLAYER_H
