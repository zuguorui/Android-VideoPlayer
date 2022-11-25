//
// Created by 祖国瑞 on 2022/9/5.
//

#ifndef ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H
#define ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H

#include <stdlib.h>
#include "VideoFrame.h"
#include "PlayerContext.h"

class IVideoOutput {
public:
    IVideoOutput(PlayerContext *playerContext) {
        this->playerCtx = playerContext;
    };
    virtual bool create() = 0;
    virtual void release() = 0;
    virtual void setWindow(void *window) = 0;
    virtual void setSize(int32_t width, int32_t height) = 0;
    virtual bool isReady() = 0;
    virtual void write(VideoFrame* frame) = 0;

protected:
    PlayerContext *playerCtx;
};


#endif //ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H
