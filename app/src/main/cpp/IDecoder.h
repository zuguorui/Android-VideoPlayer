//
// Created by 祖国瑞 on 2022/9/19.
//

#ifndef ANDROID_VIDEOPLAYER_IDECODER_H
#define ANDROID_VIDEOPLAYER_IDECODER_H

#include <iostream>
#include <stdlib.h>
#include "CodecType.h"
#include "PreferCodecType.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
}

enum CodecState {
    OK, AGAIN, IS_EOF, NOT_INIT, UNKNOWN_ERROR
};

class IDecoder {
public:
    virtual const char* getName() = 0;
    virtual bool init(AVCodecParameters *params, PreferCodecType preferType) = 0;
    virtual void release() = 0;
    virtual int sendPacket(const AVPacket *packet) = 0;
    virtual int receiveFrame(AVFrame *frame) = 0;
    virtual void flush() = 0;
    virtual CodecType getCodecType() = 0;
    virtual AVPixelFormat getPixelFormat() = 0;
};

#endif //ANDROID_VIDEOPLAYER_IDECODER_H
