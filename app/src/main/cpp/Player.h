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

#include "Log.h"
#include "VideoFrame.h"
#include "AudioFrame.h"
#include "PlayerContext.h"
#include "IDecoder.h"
#include "FFmpegDecoder.h"
#include "output/IVideoOutput.h"
#include "output/IAudioOutput.h"
#include "StreamInfo.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
};

class Player {

public:
    Player();
    Player(Player&) = delete;
    Player(Player&&) = delete;
    ~Player();

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

    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;

    AVFormatContext *formatCtx = nullptr;

    std::string filePath = "";

    int audioStreamIndex = -1;
    int videoStreamIndex = -1;

    std::map<int, StreamInfo> audioStreamMap;
    std::map<int, StreamInfo> videoStreamMap;

    IDecoder *findHWDecoder(AVCodecParameters *params);
    IDecoder *findSWDecoder(AVCodecParameters *params);

    void findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, IDecoder **decoder, int *streamIndex);




};


#endif //ANDROID_VIDEOPLAYER_PLAYER_H
