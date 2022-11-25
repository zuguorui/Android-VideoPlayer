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
};

// RGB video frame
struct VideoFrame {
    uint8_t *data;
    int64_t pts;
    int width;
    int height;


    VideoFrame(size_t capacity) {
        this->capacity = capacity;
        data = (uint8_t *)malloc(capacity * sizeof(float));
    }

    ~VideoFrame() {
        free(data);
    }

    size_t getCapacity() {
        return capacity;
    }

    static VideoFrame *alloc(AVPixelFormat pixelFormat, int width, int height);

private:
    size_t capacity;
};

VideoFrame *VideoFrame::alloc(AVPixelFormat pixelFormat, int width, int height) {
    int capacity = -1;
    switch (pixelFormat) {
        case AV_PIX_FMT_RGB24:
            capacity = 3 * width * height;
            break;
        case AV_PIX_FMT_ARGB:
            capacity = 4 * width * height;
            break;
        case AV_PIX_FMT_YUV420P:

    }
}

#endif //ANDROID_VIDEOPLAYER_VIDEOFRAME_H
