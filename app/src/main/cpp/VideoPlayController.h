//
// Created by 祖国瑞 on 2020-04-11.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H
#define ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>

#include "IAudioFrameProvider.h"
#include "IVideoFrameProvider.h"
#include "IMediaDataReceiver.h"
#include "BlockRecyclerQueue.h"
#include "VideoFileDecoder.h"
#include "IAudioPlayer.h"
#include "IVideoPlayer.h"
#include "OpenGLESPlayer.h"
#include "OpenSLESPlayer.h"


using namespace std;

class VideoPlayController: public IMediaDataReceiver, public IAudioFrameProvider, public IVideoFrameProvider{
public:
    VideoPlayController();
    ~VideoPlayController();

    bool openFile(const char *path);

    void closeFile();

    void start();

    void stop();

    void seek(int64_t posMS);


    void receiveAudioFrame(AudioFrame *audioData) override;

    void receiveVideoFrame(VideoFrame *videoData) override;

    AudioFrame *getUsedAudioFrame() override;

    VideoFrame *getUsedVideoFrame() override;

    void putUsedAudioFrame(AudioFrame *audioData) override;

    void putUsedVideoFrame(VideoFrame *videoData) override;

    AudioFrame *getAudioFrame() override;

    void putBackUsed(AudioFrame *data) override;

    VideoFrame *getVideoFrame() override;

    void putBackUsed(VideoFrame* data) override;

private:

    VideoFileDecoder *decoder = NULL;

    IAudioPlayer *audioPlayer = NULL;
    IVideoPlayer *videoPlayer = NULL;

    BlockRecyclerQueue<AudioFrame *> *audioQueue = NULL;
    BlockRecyclerQueue<VideoFrame *> *videoQueue = NULL;

    mutex audioMu;
    mutex videoMu;

    unique_lock<mutex> *audioLock = NULL;
    unique_lock<mutex> *videoLock = NULL;

    condition_variable allowGetVideoSignal;

    bool allowGetVideoFlag = false;

    VideoFrame *nextVideoFrame = NULL;

    int64_t currentPositionMS = 0;
};


#endif //ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H
