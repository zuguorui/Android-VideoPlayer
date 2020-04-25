//
// Created by incus on 2020-04-15.
//

#include "OpenSLESPlayer.h"
#include <android/log.h>
#define MODULE_NAME  "OpenSLESPlayer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

FILE *pcmFile;

OpenSLESPlayer::OpenSLESPlayer() {
//    pcmFile = fopen("/sdcard/video.pcm", "wb");
}

OpenSLESPlayer::~OpenSLESPlayer() {
//    fclose(pcmFile);
}

void OpenSLESPlayer::processAudio() {
    unique_lock<mutex> providerLock(providerMu);
    if(provider != NULL)
    {
        AudioFrame *data = provider->getAudioFrame();
        if(data == NULL)
        {
            LOGE("get a NULL audio frame");
            (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, EMPTY_BUFFER_SAMPLES * 2 * sizeof(int16_t));
        } else
        {
//            LOGD("audio frame sample count = %d, pts = %ld", data->sampleCount, data->pts);
            memcpy(emptyBuffer, data->data, data->sampleCount * 2 * sizeof(int16_t));
            (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, data->sampleCount * 2 * sizeof(int16_t));
//            fwrite(data->data, sizeof(int16_t), data->sampleCount * 2, pcmFile);
            provider->putBackUsed(data);
        }


    }
    providerLock.unlock();
}

void OpenSLESPlayer::audioCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    OpenSLESPlayer *player = (OpenSLESPlayer *)context;
    player->processAudio();
}

bool OpenSLESPlayer::create() {
    SLresult result;

    //create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    SLInterfaceID ids1[1] = {SL_IID_OUTPUTMIX};
    SLboolean reqs1[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, ids1, reqs1);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    // Create player
    SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcmFormat = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                  SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&bufferQueue, &pcmFormat};

    SLDataLocator_OutputMix locOutputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSink = {&locOutputMix, NULL};

    SLInterfaceID ids2[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    SLboolean reqs2[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_FALSE};

    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSink, 2, ids2, reqs2);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerObject)->GetInterface(playerObject, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


    result = (*playerBufferQueue)->RegisterCallback(playerBufferQueue, audioCallback, this);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }


//    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &bqPlayerVolume);
//    if(result != SL_RESULT_SUCCESS)
//    {
//        return false;
//    }


    result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }



    (*playerBufferQueue)->Enqueue(playerBufferQueue, emptyBuffer, EMPTY_BUFFER_SAMPLES * 2 * sizeof(int16_t));

    return true;
}

void OpenSLESPlayer::release() {
    if(playerPlay != NULL)
    {
        (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        playerPlay = NULL;
        (*playerObject)->Destroy(playerObject);
        playerObject = NULL;
        playerBufferQueue = NULL;

    }
    if(engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);
    }
}

void OpenSLESPlayer::start() {
    if(playerPlay == NULL)
    {
        return;
    }

    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);

    if(result != SL_RESULT_SUCCESS)
    {
        LOGE("start play failed");
    }
    return;
}

void OpenSLESPlayer::stop() {
    if(playerPlay == NULL)
    {
        return;
    }
    SLresult result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
    if(result != SL_RESULT_SUCCESS)
    {
        LOGE("stop play failed");
    }
    LOGD("stop play");
    return;
}

void OpenSLESPlayer::setAudioFrameProvider(IAudioFrameProvider *provider) {
    unique_lock<mutex> providerLock(providerMu);
    this->provider = provider;
    providerLock.unlock();
}

void OpenSLESPlayer::removeAudioFrameProvider(IAudioFrameProvider *provider) {
    unique_lock<mutex> providerLock(providerMu);
    if(this->provider == NULL || provider == NULL)
    {
        return;
    }
    if(this->provider == provider)
    {
        removeAudioDataProviderFlag = true;
    }
    providerLock.unlock();
}

bool OpenSLESPlayer::isPlaying() {
    if(playerPlay == NULL)
    {
        return false;
    }
    SLuint32 state;
    SLresult result = (*playerPlay)->GetPlayState(playerPlay, &state);
    if(result != SL_RESULT_SUCCESS)
    {
        return false;
    }
    return state == SL_PLAYSTATE_PLAYING;
}
