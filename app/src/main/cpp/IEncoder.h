//
// Created by zu on 2024/8/10.
//

#pragma once

#include <iostream>
#include <stdlib.h>
#include "CodecType.h"
#include "PreferCodecType.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
}

class IEncoder {
public:
    virtual ~IEncoder() {}
    virtual const char* getName() = 0;
    virtual bool init(AVCodecParameters *params, PreferCodecType preferType) = 0;
    virtual void release() = 0;
    virtual int receivePacket(AVPacket *packet) = 0;
    virtual int sendFrame(const AVFrame *frame) = 0;
    virtual void flush() = 0;
    virtual CodecType getCodecType() = 0;
    virtual AVPixelFormat getPixelFormat() = 0;
};
