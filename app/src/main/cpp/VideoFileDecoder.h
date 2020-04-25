//
// Created by 祖国瑞 on 2020-04-12.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOFILEDECODER_H
#define ANDROID_VIDEOPLAYER_VIDEOFILEDECODER_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <thread>
#include "IAudioFrameProvider.h"
#include "IVideoFrameProvider.h"
#include "IMediaDataReceiver.h"
#include "BlockRecyclerQueue.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

using namespace std;

class VideoFileDecoder{
public:
    VideoFileDecoder();
    ~VideoFileDecoder();

    bool openFile(const char *inputFile);
    void closeInput();
    bool hasAudio();
    bool hasVideo();

    void seekTo(int64_t position);//ms

    void setDataReceiver(IMediaDataReceiver *receiver);
    void removeDataReceiber(IMediaDataReceiver *receiver);

    int32_t getVideoWidth();
    int32_t getVideoHeight();

    int64_t getDuration();

private:
    bool initComponents(const char *path);
    void resetComponents();

    void decodeAudio();
    void decodeVideo();
    void readFile();

    AVPacket *getFreePacket();

    static void audioThreadCallback(void *context);
    static void videoThreadCallback(void *context);
    static void readThreadCallback(void *context);

    void recyclePackets();

    void discardAllReadPackets();

    AVFormatContext *formatCtx = NULL;

    int audioIndex = -1, videoIndex = -1;
    AVStream *audioStream = NULL, *videoStream = NULL;
    AVCodecContext *audioCodecCtx = NULL, *videoCodecCtx = NULL;
    AVCodec *audioCodec, *videoCodec = NULL;
    SwrContext *audioSwrCtx = NULL;
    SwsContext *videoSwsCtx = NULL;

    int64_t duration = 0;

    bool seekReq = false;
    int64_t seekPosition = 0;

    IMediaDataReceiver *dataReceiver = NULL;

    thread *audioDecodeThread = NULL;
    thread *videoDecodeThread = NULL;
    thread *readThread = NULL;

    bool stopDecodeFlag = false;

    float videoFPS = 0;

    int32_t audioSampleCountLimit = 0;

    static const int32_t AUDIO_SAMPLE_RATE = 44100;

    bool audioDecodeFinished = false;
    bool videoDecodeFinished = false;

    BlockRecyclerQueue<AVPacket *> *audioPacketQueue;
    BlockRecyclerQueue<AVPacket *> *videoPacketQueue;

    mutex componentsMu;


};


#endif //ANDROID_VIDEOPLAYER_VIDEOFILEDECODER_H
