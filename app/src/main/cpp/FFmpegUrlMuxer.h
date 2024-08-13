//
// Created by zu on 2024/8/10.
//
#pragma once1

#include "FFmpegMuxer.h"

class FFmpegUrlMuxer {
public:
    FFmpegUrlMuxer();
    FFmpegUrlMuxer(const FFmpegUrlMuxer &src) = delete;
    FFmpegUrlMuxer(FFmpegUrlMuxer &&src) = delete;

    void setUrl(const char* url);

    void setOutputFormat(const char *fmt);

    int addStream(AVCodecParameters *parameters);

    void setCSD(uint8_t *buffer, int size, int streamIndex);

    bool start();

    void stop();

    void sendData(uint8_t *data, int size, int64_t pts, bool keyFrame, int streamIndex);

private:
    FFmpegMuxer muxer;
    AVIOContext *ioContext = nullptr;
    char *url = nullptr;
};


