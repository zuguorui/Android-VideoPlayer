//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_PLAYERCONTEXT_H
#define ANDROID_VIDEOPLAYER_PLAYERCONTEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "VideoFrame.h"
#include "AudioFrame.h"
#include "LinkedBlockingQueue.h"

class PlayerContext {
public:
    PlayerContext();
    ~PlayerContext();

    std::unique_ptr<VideoFrame> getEmptyVideoFrame(int64_t capacity);
    void recycleVideoFrame(std::unique_ptr<VideoFrame> &videoFrame);

    std::unique_ptr<AudioFrame> getEmptyAudioFrame(int64_t capacity);
    void recycleAudioFrame(std::unique_ptr<AudioFrame> &audioFrame);

private:

    LinkedBlockingQueue<VideoFrame *> recycledVideoFrameQueue = LinkedBlockingQueue<VideoFrame *>(-1);
    LinkedBlockingQueue<AudioFrame *> recycledAudioFrameQueue = LinkedBlockingQueue<AudioFrame *>(-1);



};


#endif //ANDROID_VIDEOPLAYER_PLAYERCONTEXT_H
