//
// Created by 祖国瑞 on 2020-02-12.
//

#ifndef OPENGLTEST_EGLCORE_H
#define OPENGLTEST_EGLCORE_H

#include <stdlib.h>
#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

class EGLCore {
public:
    EGLCore();
    ~EGLCore();

    bool init();
    bool init(EGLContext sharedContext);

    void release();

    EGLDisplay getDisplay();
    EGLContext getContext();

    EGLSurface createWindowSurface(ANativeWindow *window);
    EGLSurface createOffScreenSurface(int width, int height);
    void releaseSurface(EGLSurface surface);

    bool makeCurrent(EGLSurface surface);

    bool swapBuffers(EGLSurface surface);


private:
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLConfig config;
    EGLint numConfigs;
};


#endif //OPENGLTEST_EGLCORE_H
