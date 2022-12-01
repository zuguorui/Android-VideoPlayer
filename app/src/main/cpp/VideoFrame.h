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
    int width;
    int height;
    AVPixelFormat pixelFormat;
    AVFrame *avFrame;
    int64_t pts;
    int64_t duration;


    VideoFrame(AVFrame *avFrame, AVPixelFormat avPixelFormat) {
        this->avFrame = avFrame;
        this->pixelFormat = avPixelFormat;
        initParams();
    }

    VideoFrame(VideoFrame &src) = delete;

    VideoFrame(VideoFrame &&src) {
        this->avFrame = src.avFrame;
        this->pixelFormat = src.pixelFormat;
        src.avFrame = nullptr;
        initParams()
    }

    ~VideoFrame() {
        if (avFrame) {
            av_frame_unref(avFrame);
            avFrame = nullptr;
        }
    }

private:
    void initParams() {
        if (avFrame == nullptr) {
            return;
        }
        width = avFrame->width;
        height = avFrame->height;
        pts = avFrame->pts;
        duration = avFrame->pkt_duration;
    }

};


#endif //ANDROID_VIDEOPLAYER_VIDEOFRAME_H
