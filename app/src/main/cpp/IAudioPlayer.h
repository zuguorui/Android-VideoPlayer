//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_IAUDIOPLAYER_H
#define ANDROID_VIDEOPLAYER_IAUDIOPLAYER_H

#include <iostream>
#include <stdlib.h>

#include "IAudioFrameProvider.h"

class IAudioPlayer {
public:
    virtual bool create() = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool isPlaying() = 0;
    virtual void setAudioFrameProvider(IAudioFrameProvider *provider) = 0;
    virtual void removeAudioFrameProvider(IAudioFrameProvider *provider) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IAUDIOPLAYER_H
