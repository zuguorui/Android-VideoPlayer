//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_IVIDEOPLAYER_H
#define ANDROID_VIDEOPLAYER_IVIDEOPLAYER_H

#include <stdlib.h>
#include "IVideoFrameProvider.h"

class IVideoPlayer {
public:
    virtual bool create() = 0;
    virtual void release() = 0;
    virtual void refresh() = 0;
    virtual void setVideoFrameProvider(IVideoFrameProvider *provider) = 0;
    virtual void removeVideoFrameProvider(IVideoFrameProvider *provider) = 0;
    virtual void setWindow(void *window) = 0;
    virtual void setSize(int32_t width, int32_t height) = 0;
    virtual bool isReady() = 0;
};


#endif //ANDROID_VIDEOPLAYER_IVIDEOPLAYER_H
