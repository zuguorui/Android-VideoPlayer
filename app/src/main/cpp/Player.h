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

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
};

class Player {

public:
    Player();
    Player(Player&) = delete;
    Player(Player&&) = delete;
    ~Player();

    bool initVideoOutput(void *window);

    bool initAudioOutput(int32_t sampleRate, int32_t channels);

    bool openFile(std::string pathStr);

    void release();

    bool play();

    void pause();

    int64_t getDurationMS();

    int64_t getCurrentPtsMS();

    bool seek(int64_t ptsMS);


private:
    IDecoder *videoDecoder = nullptr;
    IDecoder *audioDecoder = nullptr;

    PlayerContext playerContext;

    IVideoOutput *videoOutput = nullptr;
    IAudioOutput *audioOutput = nullptr;
    AudioConverter audioConverter;

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

    std::thread *decodeAudioThread = nullptr;
    std::thread *decodeVideoThread = nullptr;
    std::thread *syncThread = nullptr;

    std::atomic_bool stopFlag = false;

    void findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, IDecoder **decoder, int *streamIndex);

    static void decodeAudioCallback(void *context);
    void decodeAudioLoop();

    static void decodeVideoCallback(void *context);
    void decodeVideoLoop();

    static void readPacketCallback(void *context);
    void readPacketLoop();

    static void syncCallback(void *context);
    void syncLoop();

    AudioFrame *convertAudioFrame(AVFrame *src);
    VideoFrame *convertVideoFrame(AVFrame *src);

};


#endif //ANDROID_VIDEOPLAYER_PLAYER_H
