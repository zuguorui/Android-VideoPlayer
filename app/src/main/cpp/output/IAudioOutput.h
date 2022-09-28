//
// Created by 祖国瑞 on 2022/9/5.
//

#ifndef ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H
#define ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H

#include <stdlib.h>
#include "PlayerContext.h"
#include "AudioFrame.h"
extern "C" {
#include "FFmpeg/libavformat/avformat.h"
};

class IAudioOutput {
public:
    IAudioOutput(PlayerContext *playerContext) {
        this->playerCtx = playerContext;
    }

    virtual bool create(int sampleRate, int channels, AVSampleFormat sampleFormat) {
        this->sampleRate = sampleRate;
        this->channels = channels;
        this->sampleFormat = sampleFormat;
        return true;
    };

    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void write(AudioFrame *audioFrame) = 0;
    virtual void write(uint8_t *buffer, int framesPerChannel) = 0;

    int getSampleRate() {
        return sampleRate;
    }

    int getChannels() {
        return channels;
    }

    AVSampleFormat getSampleFormat() {
        return sampleFormat;
    }

protected:
    PlayerContext *playerCtx;
    int sampleRate;
    int channels;
    AVSampleFormat sampleFormat;
};

#endif //ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H
