//
// Created by 祖国瑞 on 2022/9/7.
//

#include "PlayerContext.h"

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
    recycledAudioFrameQueue.push(audioFrame);
}

VideoFrame* PlayerContext::getEmptyVideoFrame() {
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
    recycledVideoFrameQueue.push(videoFrame);
}
