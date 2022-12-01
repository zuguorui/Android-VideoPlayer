//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#define ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

struct AudioFrame {
    int64_t pts;
    size_t channels;
    size_t framesPerChannel;
    AVSampleFormat sampleFormat;
    AVFrame *avFrame;

    AudioFrame(AVFrame *avFrame, AVSampleFormat sampleFormat) {
        this->avFrame = avFrame;
        this->sampleFormat = sampleFormat;
        initParams();
    }

    AudioFrame(AudioFrame &src) = delete;

    AudioFrame(AudioFrame &&src) {
        this->avFrame = src.avFrame;
        this->sampleFormat = src.sampleFormat;
        initParams();
    }

    ~AudioFrame() {
        if (avFrame) {
            av_frame_unref(avFrame);
            avFrame = nullptr;
        }
    }

private:
    void initParams() {
        pts = avFrame->pts;
        channels = avFrame->channels;
        framesPerChannel = avFrame->nb_samples;
    }

};
#endif //ANDROID_VIDEOPLAYER_AUDIOFRAME_H
