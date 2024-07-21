#include <jni.h>
#include <string>
#include <iostream>
#include <stdlib.h>

#include "JavaStateListener.h"

#include "Player.h"
#include "Log.h"

using namespace std;

#define TAG "native_lib"


ANativeWindow *window = nullptr;
JavaStateListener *stateListener = nullptr;
Player *player = nullptr;

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nInit(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nInit");
    if (player != nullptr) {
        return;
    }
    player = new Player();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nDestroy(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nDestroy");
    if (player == nullptr) {
        return;
    }
    player->pause();
    delete(player);
    player = nullptr;

    if (stateListener != nullptr) {
        delete(stateListener);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nStart(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nStart");
    if(player == nullptr)
    {
        LOGE(TAG, "player is NULL when call nStart");
        return;
    }
    player->play();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nStop(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nStop");
    if(player == nullptr)
    {
        LOGE(TAG, "player is NULL when call nStop");
        return;
    }
    player->pause();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_zu_videoplayer_PlayActivity_nOpenFile(JNIEnv *env, jobject instance, jstring filePath)
{
    LOGD(TAG, "nOpenFile");
    if(player == nullptr)
    {
        LOGE(TAG, "controller is NULL when call nOpenFile");
        return false;

    }
    const char *pathChars = env->GetStringUTFChars(filePath, NULL);

    bool result = player->openFile(pathChars);

    env->ReleaseStringUTFChars(filePath, pathChars);

    return result;
}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSetSurface(JNIEnv *env, jobject instance, jobject surface)
{
    LOGD(TAG, "call nSetSurface");
    if(player == nullptr)
    {
        LOGE(TAG, "player is NULL when call nSetSurface");
        return;
    }
    window = ANativeWindow_fromSurface(env, surface);
    player->setWindow(window);
}



extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSetSize(JNIEnv *env, jobject instance, jint width, jint height)
{
    LOGD(TAG, "call nSetSize");
    if(player == NULL)
    {
        LOGE(TAG, "controller is NULL when call nSetSize");
        return;
    }
    player->setScreenSize(width, height);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_zu_videoplayer_PlayActivity_nGetDuration(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nGetDuration");
    if(player == NULL)
    {
        LOGE(TAG, "player is NULL when call nSetSize");
        return 0;
    }
    return player->getDurationMS();
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSetPlayStateListener(JNIEnv *env, jobject instance, jobject listener)
{
    LOGD(TAG, "call nSetPlayStateListener");
    if(player == NULL)
    {
        LOGE(TAG, "player is NULL when call nSetSize");
        return;
    }
    if (stateListener == nullptr) {
        stateListener = new JavaStateListener(env, listener);
    }

    player->setPlayStateListener(stateListener);
}

extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nRemovePlayStateListener(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nRemovePlayStateListener");
    if(player == NULL)
    {
        LOGE(TAG, "player is NULL when call nSetSize");
        return;
    }

    player->removePlayStateListener();
    if(stateListener)
    {
        delete(stateListener);
        stateListener = nullptr;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_zu_videoplayer_PlayActivity_nIsPlaying(JNIEnv *env, jobject instance)
{
    LOGD(TAG, "call nIsPlaying");
    if(player == NULL)
    {
        LOGE(TAG, "player is NULL when call nSetSize");
        return false;
    }

    return player->isPlaying();

}


extern "C" JNIEXPORT void JNICALL
Java_com_zu_videoplayer_PlayActivity_nSeek(JNIEnv *env, jobject instance, jlong position)
{
    LOGD(TAG, "call nSeek");
    if(player == NULL)
    {
        LOGE(TAG, "controller is NULL when call nSetSize");
        return;
    }
    player->seek(position);
}







extern "C" JNIEXPORT jstring JNICALL
Java_com_zu_videoplayer_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
