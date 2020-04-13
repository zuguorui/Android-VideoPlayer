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

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/mem.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

using namespace std;

class VideoFileDecoder{
public:
    VideoFileDecoder();
    ~VideoFileDecoder();
    void openFile(const char *inputFile);
    void closeInput();
    bool hasAudio();
    bool hasVideo();

    void seekTo(int64_t position);//ms

    void setDataReceiver(IMediaDataReceiver *receiver);
    void removeDataReceiber(IMediaDataReceiver *receiver);

private:
    bool initComponents(const char *path);
    void resetComponents();

    void decodeAudio();
    void decodeVideo();

    static void *audioThreadCallback(void *context);
    static void *videoThreadCallback(void *context);

    AVFormatContext *formatCtx = NULL;

    int audioIndex = -1, videoIndex = -1;
    AVStream *audioStream = NULL, *videoStream = NULL;
    AVCodecContext *audioCodecCtx = NULL, *videoCodecCtx = NULL;
    AVCodec *audioCodec, *videoCodec = NULL;
    SwrContext *audioSwrCtx = NULL, *videoSwrCtx = NULL;

    int64_t duration = 0;

    int64_t audioSeekPosition = 0;
    int64_t videoSeekPosition = 0;
    bool seekAudioReq = false;
    bool seekVideoReq = false;

    IMediaDataReceiver *dataReceiver = NULL;

    thread *audioDecodeThread = NULL;
    thread *videoDecodeThread = NULL;

    bool stopDecodeFlag = false;

    float videoFPS = 0;

    int32_t audioSampleCountLimit = 0;

    static const int32_t AUDIO_SAMPLE_RATE = 44100;


};


#endif //ANDROID_VIDEOPLAYER_VIDEOFILEDECODER_H
