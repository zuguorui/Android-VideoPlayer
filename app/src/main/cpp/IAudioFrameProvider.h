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

struct AudioFrame{
    int64_t pts;
    int16_t *data;
    int32_t sampleCount;

    int32_t dataLenInByte = 0;

    AudioFrame(int32_t dataLenInByte)
    {
        this->dataLenInByte = dataLenInByte;
        pts = 0;
        sampleCount = 0;
        data = (int16_t *)malloc(dataLenInByte);
        memset(data, 0, dataLenInByte);
    }

    ~AudioFrame(){
        if(data != NULL)
        {
            free(data);
        }
    }
};

class IAudioFrameProvider {
public:
    virtual AudioFrame* getAudioFrame() = 0;
    virtual void putBackUsed(AudioFrame *data) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IAUDIOFRAMEPROVIDER_H
