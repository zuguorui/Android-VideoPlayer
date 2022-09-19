//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOFRAME_H
#define ANDROID_VIDEOPLAYER_VIDEOFRAME_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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

private:
    size_t capacity;
};

#endif //ANDROID_VIDEOPLAYER_VIDEOFRAME_H
