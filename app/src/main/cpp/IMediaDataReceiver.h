//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_IMEDIADATARECEIVER_H
#define ANDROID_VIDEOPLAYER_IMEDIADATARECEIVER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "IVideoFrameProvider.h"
#include "IAudioFrameProvider.h"

class IMediaDataReceiver {
public:
    virtual void receiveAudioFrame(AudioFrame *audioData) = 0;
    virtual void receiveVideoFrame(VideoFrame *videoData) = 0;
    virtual AudioFrame* getUsedAudioFrame() = 0;
    virtual VideoFrame* getUsedVideoFrame() = 0;

    virtual void putUsedAudioFrame(AudioFrame *audioData) = 0;
    virtual void putUsedVideoFrame(VideoFrame *videoData) = 0;
};


#endif //ANDROID_VIDEOPLAYER_IMEDIADATARECEIVER_H
