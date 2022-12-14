//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_PLAYER_H
#define ANDROID_VIDEOPLAYER_PLAYER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>
#include <thread>

#include "Log.h"
#include "VideoFrame.h"
#include "AudioFrame.h"
#include "PlayerContext.h"
#include "IDecoder.h"
#include "FFmpegDecoder.h"
#include "output/IVideoOutput.h"
#include "output/IAudioOutput.h"
#include "StreamInfo.h"
#include "AudioConverter.h"
#include "android/audio/OboePlayer.h"
#include "Factory.h"
#include "IPlayStateListener.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
}

class Player {

public:
    Player();
    Player(Player&) = delete;
    Player(Player&&) = delete;
    ~Player();

    void setWindow(void *window);

    bool openFile(std::string pathStr);

    void release();

    bool play();

    void pause();

    int64_t getDurationMS();

    int64_t getCurrentPtsMS();

    bool seek(int64_t ptsMS);

    bool setScreenSize(int width, int height);

    void setPlayStateListener(IPlayStateListener *listener);

    void removePlayStateListener();

    bool isPlaying();


private:
    IDecoder *videoDecoder = nullptr;
    IDecoder *audioDecoder = nullptr;

    PlayerContext playerContext;

    IVideoOutput *videoOutput = nullptr;
    IAudioOutput *audioOutput = nullptr;

    AVFormatContext *formatCtx = nullptr;
    std::string filePath = "";

    int audioStreamIndex = -1;
    int videoStreamIndex = -1;

    std::map<int, StreamInfo> audioStreamMap;
    std::map<int, StreamInfo> videoStreamMap;

    LinkedBlockingQueue<AVPacket *> audioPacketQueue = LinkedBlockingQueue<AVPacket *>(20);
    LinkedBlockingQueue<AVPacket *> videoPacketQueue = LinkedBlockingQueue<AVPacket *>(5);

    LinkedBlockingQueue<AudioFrame *> audioFrameQueue = LinkedBlockingQueue<AudioFrame *>(20);
    LinkedBlockingQueue<VideoFrame *> videoFrameQueue = LinkedBlockingQueue<VideoFrame *>(5);

    std::thread *readStreamThread = nullptr;
    std::thread *decodeAudioThread = nullptr;
    std::thread *decodeVideoThread = nullptr;
    std::thread *syncThread = nullptr;

    std::atomic_bool stopReadFlag = false;
    std::atomic_bool stopDecodeAudioFlag = false;
    std::atomic_bool stopDecodeVideoFlag = false;
    std::atomic_bool stopSyncFlag = false;

    std::mutex readStreamMu;
    std::mutex decodeVideoMu;
    std::mutex decodeAudioMu;
    std::mutex syncMu;

    AudioFrame *unPlayedAudioFrame = nullptr;
    VideoFrame *unPlayedVideoFrame = nullptr;

    std::atomic_int64_t lastAudioPts = 0;
    std::atomic_int64_t lastVideoPts = 0;

    std::atomic_bool seekReq = false;
    std::atomic_int64_t seekPtsMS = 0;

    void *nativeWindow = nullptr;
    int screenWidth = 1920;
    int screenHeight = 1080;

    IPlayStateListener *stateListener = nullptr;

    bool createVideoOutputAfterSetWindow = false;

    bool createAudioOutput();

    bool createVideoOutput();

    void findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, IDecoder **decoder, int *streamIndex);

    static void decodeAudioCallback(void *context);
    void decodeAudioLoop();

    static void decodeVideoCallback(void *context);
    void decodeVideoLoop();

    static void readStreamCallback(void *context);
    void readStreamLoop();

    static void syncCallback(void *context);
    void syncLoop();

    void startReadStreamThread();
    void stopReadStreamThread();

    void startDecodeVideoThread();
    void stopDecodeVideoThread();

    void startDecodeAudioThread();
    void stopDecodeAudioThread();

    void startSyncThread();
    void stopSyncThread();


};


#endif //ANDROID_VIDEOPLAYER_PLAYER_H
