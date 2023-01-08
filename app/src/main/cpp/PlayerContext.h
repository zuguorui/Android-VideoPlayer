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
#include "PacketWrapper.h"

class PlayerContext {
public:
    PlayerContext();
    ~PlayerContext();

    VideoFrame* getEmptyVideoFrame();
    void recycleVideoFrame(VideoFrame *videoFrame);

    AudioFrame* getEmptyAudioFrame();
    void recycleAudioFrame(AudioFrame *audioFrame);

    PacketWrapper *getEmptyPacketWrapper();
    void recyclePacketWrapper(PacketWrapper *packetWrapper);

private:

    LinkedBlockingQueue<VideoFrame *> recycledVideoFrameQueue = LinkedBlockingQueue<VideoFrame *>(-1);
    LinkedBlockingQueue<AudioFrame *> recycledAudioFrameQueue = LinkedBlockingQueue<AudioFrame *>(-1);
    LinkedBlockingQueue<PacketWrapper *> recycledPacketWrapperQueue = LinkedBlockingQueue<PacketWrapper *>(-1);


};


#endif //ANDROID_VIDEOPLAYER_PLAYERCONTEXT_H
