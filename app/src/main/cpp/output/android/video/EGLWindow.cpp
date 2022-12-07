//
// Created by 祖国瑞 on 2022/12/7.
//

#include "EGLWindow.h"

#include "Log.h"

#define TAG "EGLWindow"

using namespace std;

static const EGLint eglConfigAttrs[] = {
        EGL_BUFFER_SIZE, 24,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
};

static const EGLint eglContextAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
};

EGLWindow::EGLWindow() {

}

EGLWindow::~EGLWindow() {
    release();
}

bool EGLWindow::create(ANativeWindow *nativeWindow) {
    release();

    try {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY) {
            LOGE(TAG, "eglGetDisplay failed, err = %d", eglGetError());
            throw "eglGetDisplay failed";
        }

        if (!eglInitialize(display, nullptr, nullptr)) {
            LOGE(TAG, "eglInitialize failed, err = %d", eglGetError());
            throw "eglInitialize failed";
        }

        if (!eglChooseConfig(display, eglConfigAttrs, &config, 1, &numConfigs)) {
            LOGE(TAG, "eglChooseConfig failed, err = %d", eglGetError());
            throw "eglChooseConfig failed";
        }

        context = eglCreateContext(display, config, nullptr, eglConfigAttrs);
        if (context == EGL_NO_CONTEXT) {
            LOGE(TAG, "eglCreateContext failed, err = %d", eglGetError());
            throw "eglCreateContext failed";
        }

        EGLint format;
        if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
            LOGE(TAG, "eglGetConfigAttrib failed, err = %d", eglGetError());
            throw "eglGetConfigAttrib failed";
        }

        ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);

        surface = eglCreateWindowSurface(display, config, nativeWindow, 0);
        if (surface == EGL_NO_SURFACE) {
            LOGE(TAG, "eglCreateWindowSurface failed, err = %d", eglGetError());
            throw "eglCreateWindowSurface failed";
        }

        if (!eglMakeCurrent(display, surface, surface, context)) {
            LOGE(TAG, "eglMakeCurrent failed, err = %d", eglGetError());
            throw "eglMakeCurrent failed";
        }

    } catch (string msg) {
        LOGE(TAG, "%s", msg.c_str());
        release();
        return false;
    }

    return true;

}

void EGLWindow::release() {
    if (surface != EGL_NO_SURFACE) {
        eglDestroySurface(display, surface);
        surface = EGL_NO_SURFACE;
    }

    if (context != EGL_NO_CONTEXT) {
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
    }

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        display = EGL_NO_DISPLAY;
    }
}

bool EGLWindow::swapBuffer() {
    if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE) {
        LOGE(TAG, "display or surface is null when swapBuffer");
        return false;
    }
    return eglSwapBuffers(display, surface);
}

bool EGLWindow::isReady() {
    return display != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE;
}

