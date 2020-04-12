//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_IAUDIOFRAMEPROVIDER_H
#define ANDROID_VIDEOPLAYER_IAUDIOFRAMEPROVIDER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

using namespace std;

typedef struct {
    int64_t pts;
    int16_t *data;
    int32_t sampleCount;
}AudioFrame;

class IAudioFrameProvider {
public:
    virtual AudioFrame* getAudioFrame() = 0;
    virtual void putbackUsed(AudioFrame *data) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IAUDIOFRAMEPROVIDER_H
