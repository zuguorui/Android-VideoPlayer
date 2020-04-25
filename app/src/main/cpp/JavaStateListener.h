//
// Created by incus-pc-2 on 2019/12/28.
//

#ifndef ANDROID_VIDEO_PLAYER_JAVASTATELISTENER_H
#define ANDROID_VIDEO_PLAYER_JAVASTATELISTENER_H

#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include "IPlayStateListener.h"

using namespace std;

class JavaStateListener: public IPlayStateListener {
public:
    JavaStateListener(JNIEnv *env, jobject listener);
    ~JavaStateListener();
//    void infoGet(int64_t duration, int32_t picBufferLen);
    void progressChanged(int64_t currentProgress, bool isPlayFinished) override;
    void playStateChanged(bool isPlay) override;
private:
//    JNIEnv *env = NULL;
    JavaVM *vm = NULL;
    jobject listener = NULL;
//    jmethodID infoGetMethod = NULL;
    jmethodID progressChangedMethod = NULL;
    jmethodID playStateChangedMethod = NULL;
};


#endif //FFMPEGAUDIOPLAYER_JAVASTATELISTENER_H
