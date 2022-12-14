//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOFRAME_H
#define ANDROID_VIDEOPLAYER_VIDEOFRAME_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

// video frame
struct VideoFrame {
    int width = -1;
    int height = -1;
    AVPixelFormat pixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;
    AVFrame *avFrame = nullptr;
    int64_t pts = -1; // ms
    int64_t durationMS = -1;
    AVRational timeBase;

    VideoFrame() {

    }

    void setParams(AVFrame *avFrame, AVPixelFormat avPixelFormat, AVRational timeBase) {
        reset();
        this->avFrame = avFrame;
        this->pixelFormat = avPixelFormat;
        this->timeBase = timeBase;
        initParams();
    }

    VideoFrame(AVFrame *avFrame, AVPixelFormat avPixelFormat, AVRational timeBase): VideoFrame() {
        setParams(avFrame, avPixelFormat, timeBase);
    }

    VideoFrame(VideoFrame &src) = delete;

    VideoFrame(VideoFrame &&src) {
        this->avFrame = src.avFrame;
        this->pixelFormat = src.pixelFormat;
        src.avFrame = nullptr;
        initParams();
    }

    ~VideoFrame() {
        if (avFrame) {
            av_frame_unref(avFrame);
            avFrame = nullptr;
        }
    }

    void reset() {
        if (avFrame) {
            av_frame_unref(avFrame);
            avFrame = nullptr;
        }
        width = -1;
        height = -1;
        pixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;
        pts = -1;
        durationMS = -1;
    }

private:
    void initParams() {
        if (avFrame == nullptr) {
            return;
        }
        width = avFrame->width;
        height = avFrame->height;
        pts = avFrame->pts * av_q2d(timeBase) * 1000;
        durationMS = avFrame->pkt_duration * av_q2d(timeBase) * 1000;
    }

};


#endif //ANDROID_VIDEOPLAYER_VIDEOFRAME_H
