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
    uint8_t *data;

    size_t channels;
    size_t framesPerChannel;

    AudioFrame(size_t capacity) {
        this->capacity = capacity;
        data = (uint8_t *)malloc(capacity);
        channels = 0;
        framesPerChannel = 0;
    }

    ~AudioFrame() {
        free(data);
    }

    size_t getCapacity() {
        return capacity;
    }

private:
    size_t capacity;

};
#endif //ANDROID_VIDEOPLAYER_AUDIOFRAME_H
