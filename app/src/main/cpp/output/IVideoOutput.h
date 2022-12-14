//
// Created by 祖国瑞 on 2022/9/5.
//

#ifndef ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H
#define ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H

#include <stdlib.h>
#include "VideoFrame.h"
#include "PlayerContext.h"
#include "SizeMode.h"



class IVideoOutput {
public:
    IVideoOutput(PlayerContext *playerContext) {
        this->playerCtx = playerContext;
    };
    virtual void setSrcFormat(AVPixelFormat pixelFormat, AVColorSpace colorSpace, bool isHDR) = 0;
    virtual bool create() = 0;
    virtual void release() = 0;
    virtual void setWindow(void *window) = 0;
    virtual void setScreenSize(int32_t width, int32_t height) = 0;
    virtual bool isReady() = 0;
    virtual void write(VideoFrame* frame) = 0;
    virtual void setSizeMode(SizeMode mode) = 0;

protected:
    PlayerContext *playerCtx;
    AVPixelFormat srcPixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;
};


#endif //ANDROID_VIDEOPLAYER_IVIDEOOUTPUT_H
