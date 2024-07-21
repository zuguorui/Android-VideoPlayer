//
// Created by 祖国瑞 on 2022/9/7.
//

#include "PlayerContext.h"
#include "Log.h"

#define TAG "PlayerContext"

using namespace std;

PlayerContext::PlayerContext() {
    recycledAudioFrameQueue.setBlockPop(false);
    recycledAudioFrameQueue.setBlockPush(false);

    recycledVideoFrameQueue.setBlockPop(false);
    recycledVideoFrameQueue.setBlockPush(false);

    recycledPacketWrapperQueue.setBlockPop(false);
    recycledPacketWrapperQueue.setBlockPush(false);
}

PlayerContext::~PlayerContext() {

}

AudioFrame* PlayerContext::getEmptyAudioFrame() {
    optional<AudioFrame *> frameOpt = recycledAudioFrameQueue.popFront(false);
    AudioFrame *frame = nullptr;
    if (frameOpt.has_value()) {
        frame = frameOpt.value();
    }

    if (frame == nullptr) {
        frame = new AudioFrame();
    }
    return frame;
}

void PlayerContext::recycleAudioFrame(AudioFrame *audioFrame) {
    audioFrame->reset();
    recycledAudioFrameQueue.pushBack(audioFrame);
}

VideoFrame* PlayerContext::getEmptyVideoFrame() {
    //LOGD(TAG, "getEmptyVideoFrame: recycler.size = %d", recycledVideoFrameQueue.getSize());
    optional<VideoFrame *> frameOpt = recycledVideoFrameQueue.popFront(false);
    VideoFrame *frame = nullptr;
    if (frameOpt.has_value()) {
        frame = frameOpt.value();
    }

    if (frame == nullptr) {
        frame = new VideoFrame();
    }
    return frame;
}

void PlayerContext::recycleVideoFrame(VideoFrame *videoFrame) {
    videoFrame->reset();
    recycledVideoFrameQueue.pushBack(videoFrame);
}

int getPacketCount = 0;

PacketWrapper *PlayerContext::getEmptyPacketWrapper() {
    //getPacketCount++;
    //LOGD(TAG, "getEmptyPacketWrapper: recycler.size = %d", recycledPacketWrapperQueue.getSize());
    optional<PacketWrapper *> opt = recycledPacketWrapperQueue.popFront(false);
    PacketWrapper *packetWrapper = nullptr;
    if (opt.has_value()) {
        packetWrapper = opt.value();
    }

    if (packetWrapper == nullptr) {
        packetWrapper = new PacketWrapper();
    }
    return packetWrapper;
}

void PlayerContext::recyclePacketWrapper(PacketWrapper *packetWrapper) {
    //LOGW(TAG, "recyclePacketWrapper: recycler.size = %d, getPacketCount = %d", recycledPacketWrapperQueue.getSize(), getPacketCount);
    //getPacketCount = 0;
    packetWrapper->reset();
    recycledPacketWrapperQueue.pushBack(packetWrapper);
}
