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

    virtual void setSrcFormat(int sampleRate, int channels, AVSampleFormat sampleFormat) {
        this->srcSampleRate = sampleRate;
        this->srcChannels = channels;
        this->srcSampleFormat = sampleFormat;
    }

    virtual bool create() = 0;

    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void write(AudioFrame *audioFrame) = 0;
    virtual void write(uint8_t *buffer, int framesPerChannel) = 0;

    int getSrcSampleRate() {
        return srcSampleRate;
    }

    int getSrcChannels() {
        return srcChannels;
    }

    AVSampleFormat getSrcSampleFormat() {
        return srcSampleFormat;
    }

protected:
    PlayerContext *playerCtx = nullptr;
    int srcSampleRate = -1;
    int srcChannels = -1;
    AVSampleFormat srcSampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
};

#endif //ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H
