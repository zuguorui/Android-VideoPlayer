//
// Created by incus on 2020-04-15.
//

#ifndef ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
#define ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H

#include <iostream>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <thread>

#include "IAudioPlayer.h"
#include "IAudioFrameProvider.h"

class OpenSLESPlayer: public IAudioPlayer{
public:
    OpenSLESPlayer();
    ~OpenSLESPlayer();

    bool create() override;

    void release() override;

    void start() override;

    void stop() override;

    bool isPlaying() override;

    void setAudioFrameProvider(IAudioFrameProvider *provider) override;

    void removeAudioFrameProvider(IAudioFrameProvider *provider) override;

private:

    static void audioCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
    void processAudio();

    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine;

    SLObjectItf outputMixObject = NULL;


    SLObjectItf playerObject = NULL;
    SLPlayItf playerPlay;

    SLAndroidSimpleBufferQueueItf playerBufferQueue;

    bool removeAudioDataProviderFlag = false;

    IAudioFrameProvider *provider = NULL;
    mutex providerMu;



    static const int EMPTY_BUFFER_SAMPLES = 512;
    int16_t emptyBuffer[2 * EMPTY_BUFFER_SAMPLES];

    AudioFrame *audioFrame = NULL;
    int audioFrameIndex = 0;
};


#endif //ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
