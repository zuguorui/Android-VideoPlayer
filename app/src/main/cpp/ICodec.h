//
// Created by 祖国瑞 on 2022/9/19.
//

#ifndef ANDROID_VIDEOPLAYER_ICODEC_H
#define ANDROID_VIDEOPLAYER_ICODEC_H

#include <iostream>
#include <stdlib.h>
#include "CodecType.h"
#include "PreferCodecType.h"
#include "CodecState.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
}


class ICodec {
public:
    virtual ~ICodec() {}
    virtual const char* getName() = 0;
    virtual bool init(AVCodecParameters *params, PreferCodecType preferType, bool isEncoder) = 0;
    virtual void release() = 0;
    // for decoder
    virtual int sendPacket(const AVPacket *packet) = 0;
    virtual int receiveFrame(AVFrame *frame) = 0;
    // for encoder
    virtual int sendFrame(const AVFrame *frame) = 0;
    virtual int receivePacket(AVPacket *packet) = 0;

    virtual void flush() = 0;
    virtual CodecType getCodecType() = 0;
    virtual AVPixelFormat getPixelFormat() = 0;
    virtual bool isEncoder() = 0;
};

#endif //ANDROID_VIDEOPLAYER_ICODEC_H
