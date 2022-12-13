//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#define ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

struct AudioFrame {
    int64_t pts = -1; // MS
    size_t channels = -1;
    size_t framesPerChannel = -1;
    AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
    AVFrame *avFrame = nullptr;
    int64_t durationMS;
    AVRational timeBase;

    AudioFrame() {

    }

    void setParams(AVFrame *avFrame, AVSampleFormat sampleFormat, AVRational timeBase) {
        reset();
        this->avFrame = avFrame;
        this->sampleFormat = sampleFormat;
        this->timeBase = timeBase;
        initParams();
    }

    AudioFrame(AVFrame *avFrame, AVSampleFormat sampleFormat, AVRational timeBase): AudioFrame() {
        setParams(avFrame, sampleFormat, timeBase);
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

    void reset() {
        pts = -1;
        channels = -1;
        framesPerChannel = -1;
        sampleFormat = AV_SAMPLE_FMT_NONE;
        if (avFrame) {
            av_frame_unref(avFrame);
            avFrame = nullptr;
        }
    }

private:
    void initParams() {
        pts = avFrame->pts * av_q2d(timeBase) * 1000;
        channels = avFrame->channels;
        framesPerChannel = avFrame->nb_samples;
        durationMS = avFrame->pkt_duration * av_q2d(timeBase) * 1000;
    }

};
#endif //ANDROID_VIDEOPLAYER_AUDIOFRAME_H
