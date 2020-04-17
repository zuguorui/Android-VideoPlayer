//
// Created by incus on 2020-04-15.
//

#include "OpenGLESPlayer.h"
#include <android/log.h>

#define MODULE_NAME "PicTexture"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

OpenGLESPlayer::OpenGLESPlayer() {
    LOGD("OpenGLESPlayer init");
    surface = EGL_NO_SURFACE;
    width = 1920;
    height = 1080;

}

OpenGLESPlayer::~OpenGLESPlayer() {
    LOGD("OpenGLESPlayer delete");
}

void* OpenGLESPlayer::threadCallback(void *self) {
    OpenGLESPlayer *player = (OpenGLESPlayer *)self;
    player->start();
}

bool OpenGLESPlayer::create() {
    renderThread = new thread(threadCallback, this);
    LOGD("create");
    return true;
}

void OpenGLESPlayer::release() {

}

void OpenGLESPlayer::start() {

}

void OpenGLESPlayer::stop() {

}

void OpenGLESPlayer::setVideoFrameProvider(IVideoFrameProvider *provider) {

}

void OpenGLESPlayer::removeVideoFrameProvider(IVideoFrameProvider *provider) {

}

void OpenGLESPlayer::setWindow(void *window) {

}
