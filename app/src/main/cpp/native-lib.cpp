#include <jni.h>
#include <string>
#include <iostream>
#include <stdlib.h>

#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "VideoPlayController.h"

using namespace std;

#define MODULE_NAME "native_lib"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

VideoPlayController *controller = NULL;
ANativeWindow *window = NULL;

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nInit(JNIEnv *env, jobject instance)
{
    controller = new VideoPlayController();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nDestroy(JNIEnv *env, jobject instance)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nDestroy");
        return;
    }
    delete(controller);
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nStart(JNIEnv *env, jobject instance)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nStart");
        return;
    }
    controller->start();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nStop(JNIEnv *env, jobject instance)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nStop");
        return;
    }
    controller->stop();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_zu_videoplayer_PlayActivity_nOpenFile(JNIEnv *env, jobject instance, jstring filePath)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nOpenFile");
        return false;

    }
    const char *pathChars = env->GetStringUTFChars(filePath, NULL);

    bool result =  controller->openFile(pathChars);

    env->ReleaseStringUTFChars(filePath, pathChars);

    return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nCloseFile(JNIEnv *env, jobject instance)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nCloseFile");
        return;
    }
    controller->closeFile();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSetSurface(JNIEnv *env, jobject instance, jobject surface)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nSetSurface");
        return;
    }
    window = ANativeWindow_fromSurface(env, surface);
    controller->setWindow(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nReleaseSurface(JNIEnv *env, jobject instance, jobject surface)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nReleaseSurface");
        return;
    }
    if(window != NULL)
    {
        ANativeWindow_release(window);
        window = NULL;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSetSize(JNIEnv *env, jobject instance, jint width, jint height)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nSetSize");
        return;
    }
    controller->setSize(width, height);
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSeek(JNIEnv *env, jobject instance, jlong position)
{
    if(controller == NULL)
    {
        LOGE("controller is NULL when call nSetSize");
        return;
    }
    controller->seek(position);
}







extern "C" JNIEXPORT jstring JNICALL
Java_com_zu_videoplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
