//
// Created by 祖国瑞 on 2022/9/5.
//

#ifndef ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H
#define ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H

#include <stdlib.h>

class IAudioOutput {
public:
    virtual bool create() = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif //ANDROID_VIDEOPLAYER_IAUDIOOUTPUT_H
