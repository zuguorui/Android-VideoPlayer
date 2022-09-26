//
// Created by 祖国瑞 on 2022/9/7.
//

#include "PlayerContext.h"
#include "Constants.h"

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

std::unique_ptr<AudioFrame> PlayerContext::getEmptyAudioFrame(int64_t capacity) {
    AudioFrame *frame = nullptr;
    optional<AudioFrame *> frameOpt;
    if (capacity <= 0) {
        frameOpt = recycledAudioFrameQueue.pop();
        if (frameOpt != nullopt) {
            frame = frameOpt.value();
        }
    } else {
        while ((frameOpt = recycledAudioFrameQueue.pop()) != nullopt) {
            AudioFrame *ptr = frameOpt.value();
            if (ptr->getCapacity() < capacity) {
                delete ptr;
            } else {
                frame = ptr;
                break;
            }
        }
    }

    if (frame == nullptr) {
        size_t finalCapacity = capacity > 0 ? capacity : (size_t)(0.5 * DEFAULT_SAMPLE_RATE * DEFAULT_CHANNELS * sizeof(float));
        frame = new AudioFrame(finalCapacity);
    }

    unique_ptr<AudioFrame> framePtr(frame);
    return move(framePtr);
}

void PlayerContext::recycleAudioFrame(std::unique_ptr<AudioFrame> &audioFrame) {
    AudioFrame *framePtr = audioFrame.release();
    recycledAudioFrameQueue.push(framePtr);
}

std::unique_ptr<VideoFrame> PlayerContext::getEmptyVideoFrame(int64_t capacity) {
    VideoFrame *frame = nullptr;
    optional<VideoFrame *> frameOpt;
    if (capacity <= 0) {
        frameOpt = recycledVideoFrameQueue.pop();
        if (frameOpt != nullopt) {
            frame = frameOpt.value();
        }
    } else {
        while ((frameOpt = recycledVideoFrameQueue.pop()) != nullopt) {
            VideoFrame *ptr = frameOpt.value();
            if (ptr->getCapacity() < capacity) {
                delete ptr;
            } else {
                frame = ptr;
                break;
            }
        }
    }

    if (frame == nullptr) {
        size_t finalCapacity = capacity > 0 ? capacity : (size_t)(DEFAULT_VIDEO_WIDTH * DEFAULT_VIDEO_HEIGHT * DEFAULT_PIX_SIZE);
        frame = new VideoFrame(finalCapacity);
    }

    unique_ptr<VideoFrame> framePtr(frame);

    return move(framePtr);
}

void PlayerContext::recycleVideoFrame(std::unique_ptr<VideoFrame> &videoFrame) {
    VideoFrame *framePtr = videoFrame.release();
    recycledVideoFrameQueue.push(framePtr);
}
