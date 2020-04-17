//
// Created by 祖国瑞 on 2020-02-12.
//

#include "EGLCore.h"

#include <android/log.h>

#define MODULE_NAME "EGLCore"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

const EGLint eglConfigAttrs[] = {
        EGL_BUFFER_SIZE, 24,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
};

EGLCore::EGLCore() {

}

EGLCore::~EGLCore() {

}

bool EGLCore::init() {
    return this->init(NULL);
}

bool EGLCore::init(EGLContext sharedContext) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(display == EGL_NO_DISPLAY)
    {
        LOGE("eglGetDisplay returns error %d", eglGetError());
        return false;
    }

    if(!eglInitialize(display, NULL, NULL))
    {
        LOGE("eglInitialize returns error %d", eglGetError());
        release();
        return false;
    }

    if(!eglChooseConfig(display, eglConfigAttrs, &config, 1, &numConfigs))
    {
        LOGE("eglChooseConfig returns error %d", eglGetError());
        release();
        return false;
    }

    const EGLint contextAttrs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
    };

    context = eglCreateContext(display, config, sharedContext, contextAttrs);
    if(!context)
    {
        LOGE("eglCreateContext returns error %d", eglGetError());
        release();
        return false;
    }

    return true;
}

void EGLCore::release() {
    if(display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    if(context != NULL)
    {
        eglDestroyContext(display, context);
    }

    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;

}

bool EGLCore::makeCurrent(EGLSurface surface) {
    return eglMakeCurrent(display, surface, surface, context);
}

void EGLCore::releaseSurface(EGLSurface surface)
{
    eglDestroySurface(display, surface);
    surface= EGL_NO_SURFACE;
}

EGLSurface EGLCore::createWindowSurface(ANativeWindow *window) {
    EGLSurface surface = EGL_NO_SURFACE;
    EGLint format;
    if(!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
    {
        LOGE("eglGetConfigAttrib returns error %d", eglGetError());
        return surface;
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    if(!(surface = eglCreateWindowSurface(display, config, window, 0)))
    {
        LOGE("eglCreateWindowSurface error %d", eglGetError());

    }
    return surface;
}

EGLSurface EGLCore::createOffScreenSurface(int width, int height) {
    EGLSurface surface = EGL_NO_SURFACE;
    EGLint PbufferAttributes[] = { EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE, EGL_NONE };
    if (!(surface = eglCreatePbufferSurface(display, config, PbufferAttributes))) {
        LOGE("eglCreatePbufferSurface() returned error %d", eglGetError());
    }
    return surface;
}

bool EGLCore::swapBuffers(EGLSurface surface) {
    return eglSwapBuffers(display, surface);
}

EGLContext EGLCore::getContext() {
    return context;
}

EGLDisplay EGLCore::getDisplay() {
    return display;
}