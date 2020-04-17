//
// Created by incus on 2020-04-15.
//

#ifndef ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
#define ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H

#include <iostream>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

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

    IAudioFrameProvider *waitProvider = NULL;

    static const int EMPTY_BUFFER_SAMPLES = 100;
    int16_t emptyBuffer[2 * EMPTY_BUFFER_SAMPLES];
};


#endif //ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
