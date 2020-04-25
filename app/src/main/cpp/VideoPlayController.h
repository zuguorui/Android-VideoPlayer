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
#include "OpenGLESPlayer2.h"
#include "OpenSLESPlayer.h"
#include "IPlayStateListener.h"


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

    void setWindow(void *window);

    void setSize(int width, int height);

    int64_t getDuration();

    bool isPlaying();

    void setPlayStateListener(IPlayStateListener *listener);

    void removePlayStateListener();

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

    void discardAllFrame();

    static void refreshThreadCallback(void *self);

    void imageRefreshLoop();

    IPlayStateListener *stateListener = NULL;
    mutex stateListenerMu;

    VideoFileDecoder *decoder = NULL;

    IAudioPlayer *audioPlayer = NULL;
    IVideoPlayer *videoPlayer = NULL;

    BlockRecyclerQueue<AudioFrame *> *audioQueue = NULL;
    BlockRecyclerQueue<VideoFrame *> *videoQueue = NULL;



    VideoFrame *nextVideoFrame = NULL;
    VideoFrame *waitVideoFrame = NULL;
    mutex videoMu;

    int64_t currentPositionMS = 0;
    mutex currentPosMu;
    condition_variable updateCurrentPosSignal;

    thread *imageRefreshThread = NULL;

    bool exitFlag = false;

    mutex seekMu;






};


#endif //ANDROID_VIDEOPLAYER_VIDEOPLAYCONTROLLER_H
