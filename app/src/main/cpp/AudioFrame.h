//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#define ANDROID_VIDEOPLAYER_AUDIOFRAME_H
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "Log.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}


struct AudioFrame {
    const char *TAG = "AudioFrame";
    int64_t pts = -1; // MS
    size_t numChannels = -1;
    size_t numFrames = -1;
    int64_t sampleRate = -1;
    AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
    AVFrame *avFrame = nullptr;
    int64_t durationMS = 0;
    AVRational timeBase;
    int32_t flags = 0;

    int64_t outputStartIndex = 0;
    int64_t outputFrameCount = 0;

    AudioFrame() {

    }

    AudioFrame(AVFrame *avFrame, AVSampleFormat sampleFormat, AVRational timeBase): AudioFrame() {
        setParams(avFrame, sampleFormat, timeBase);
    }

    // 不允许拷贝构造函数
    AudioFrame(AudioFrame &src) = delete;

    // 移动构造函数
    AudioFrame(AudioFrame &&src) {
        this->pts = src.pts;
        this->numChannels = src.numChannels;
        this->numFrames = src.numFrames;
        this->sampleRate = src.sampleRate;
        this->sampleFormat = src.sampleFormat;
        this->avFrame = src.avFrame;
        src.avFrame = nullptr;
        this->durationMS = src.durationMS;
        this->timeBase = src.timeBase;
        this->flags = src.flags;
        this->outputStartIndex = src.outputStartIndex;
        this->outputFrameCount = src.outputFrameCount;
    }

    ~AudioFrame() {
        //LOGD(TAG, "~AudioFrame");
        if (avFrame) {
            //av_frame_unref(avFrame);
            av_frame_free(&avFrame);
            avFrame = nullptr;
        }
    }

    void setParams(AVFrame *avFrame, AVSampleFormat sampleFormat, AVRational timeBase) {
        reset();
        this->avFrame = avFrame;
        this->sampleFormat = sampleFormat;
        this->timeBase = timeBase;
        this->sampleRate = avFrame->sample_rate;

        pts = avFrame->pts * av_q2d(timeBase) * 1000;
        numChannels = avFrame->channels;
        numFrames = avFrame->nb_samples;
        durationMS = (int64_t)(avFrame->pkt_duration * av_q2d(timeBase) * 1000);
    }

    void reset() {
        pts = -1;
        numChannels = -1;
        numFrames = -1;
        sampleFormat = AV_SAMPLE_FMT_NONE;
        outputStartIndex = 0;
        outputFrameCount = 0;
        flags = 0;
        if (avFrame) {
            av_frame_free(&avFrame);
            avFrame = nullptr;
        }
    }

    int64_t getOutputPts() {
        if (sampleRate <= 0) {
            return -1;
        }

        int64_t ptsOffset = (int64_t)(outputStartIndex * 1.0f / sampleRate * 1000);
        return pts + ptsOffset;
    }


};
#endif //ANDROID_VIDEOPLAYER_AUDIOFRAME_H
