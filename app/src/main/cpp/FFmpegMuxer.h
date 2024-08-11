//
// Created by zu on 2024/8/10.
//

#pragma once

#include <stdlib.h>

#include "AudioParameter.h"
#include "VideoParameter.h"
#include <android/native_window.h>
#include <vector>
#include <mutex>

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavformat/avio.h"
#include "FFmpeg/libavutil/avutil.h"
#include "FFmpeg/libavcodec/mediacodec.h"
}

class FFmpegMuxer {
public:

    FFmpegMuxer();
    FFmpegMuxer(const FFmpegMuxer &src) = delete;
    FFmpegMuxer(FFmpegMuxer &&src) = delete;

    ~FFmpegMuxer();

    bool start();
    void stop();

    bool isStarted();

    void setOutput(AVIOContext *ioCtx);

    void setOutputFormat(const char* fmt);

    int addStream(AVCodecParameters *parameters);

    void sendPacket(AVPacket *packet);

    void sendData(uint8_t *data, int size, int ptsMS, int streamIndex);

private:
    AVFormatContext *formatContext = nullptr;
    AVIOContext *ioContext = nullptr;
    char *outputFormat = nullptr;
    AVPacket *tempPacket = nullptr;

    std::vector<AVCodecParameters *> streamParameters;

    std::recursive_mutex sendDataMu;
};

