//
// Created by zu on 2024/8/17.
//

#pragma once

#include <stdlib.h>

#include "AudioParameter.h"
#include "VideoParameter.h"
#include <android/native_window.h>
#include <vector>
#include <mutex>
#include "FFmpegMuxer.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavformat/avio.h"
#include "FFmpeg/libavutil/avutil.h"
#include "FFmpeg/libavcodec/mediacodec.h"
#include "librtmp/rtmp.h"
#include "librtmp/log.h"
}

class RtmpPusher {
public:
    RtmpPusher();
    RtmpPusher(const RtmpPusher &src) = delete;
    RtmpPusher(RtmpPusher &&src) = delete;
    ~RtmpPusher();

    void setUrl(const char *url);

    void setOutputFormat(const char *fmt);

    int addStream(AVCodecParameters *parameters);

    void setCSD(uint8_t *buffer, int size, int streamIndex);

    bool start();

    void stop();

    void sendData(uint8_t *data, int size, int64_t pts, bool keyFrame, int streamIndex);


private:
    RTMP *rtmp = nullptr;
    char *url = nullptr;
    FFmpegMuxer muxer;
    AVIOContext *ioCtx = nullptr;

    const static int BUFFER_SIZE = 1024 * 1024;

    static int writeCallback(void *opaque, uint8_t *buf, int bufSize);

    bool initRtmp();
    void releaseRtmp();
};


