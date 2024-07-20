//
// Created by 祖国瑞 on 2022/9/7.
//

#include "PlayerContext.h"
#include "Log.h"

#define TAG "PlayerContext"

using namespace std;

PlayerContext::PlayerContext() {
    recycledAudioFrameQueue.setBlockingPop(false);
    recycledAudioFrameQueue.setBlockingPush(false);

    recycledVideoFrameQueue.setBlockingPop(false);
    recycledVideoFrameQueue.setBlockingPush(false);
}

PlayerContext::~PlayerContext() {
    optional<void *> frameOpt;
    while ((frameOpt = recycledVideoFrameQueue.pop()) != nullopt) {
        VideoFrame *ptr = (VideoFrame *)frameOpt.value();
        delete ptr;
    }

    while ((frameOpt = recycledAudioFrameQueue.pop()) != nullopt) {
        AudioFrame *ptr = (AudioFrame *)frameOpt.value();
        delete ptr;
    }
}

AudioFrame* PlayerContext::getEmptyAudioFrame() {
    optional<AudioFrame *> frameOpt = recycledAudioFrameQueue.pop(false);
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
    recycledAudioFrameQueue.push(audioFrame);
}

VideoFrame* PlayerContext::getEmptyVideoFrame() {
    //LOGD(TAG, "getEmptyVideoFrame: recycler.size = %d", recycledVideoFrameQueue.getSize());
    optional<VideoFrame *> frameOpt = recycledVideoFrameQueue.pop(false);
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
    recycledVideoFrameQueue.push(videoFrame);
}

int getPacketCount = 0;

PacketWrapper *PlayerContext::getEmptyPacketWrapper() {
    //getPacketCount++;
    //LOGD(TAG, "getEmptyPacketWrapper: recycler.size = %d", recycledPacketWrapperQueue.getSize());
    optional<PacketWrapper *> opt = recycledPacketWrapperQueue.pop(false);
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
    recycledPacketWrapperQueue.push(packetWrapper);
}
