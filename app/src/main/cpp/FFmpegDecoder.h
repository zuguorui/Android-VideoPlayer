//
// Created by 祖国瑞 on 2022/9/19.
//

#ifndef ANDROID_VIDEOPLAYER_FFMPEGDECODER_H
#define ANDROID_VIDEOPLAYER_FFMPEGDECODER_H

#include "IDecoder.h"
#include "platform.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
#include "FFmpeg/libavutil/hwcontext.h"
};

class FFmpegDecoder: public IDecoder{
public:

    FFmpegDecoder();

    ~FFmpegDecoder();

    const char* getName();

    bool init(AVCodecParameters *params, PreferCodecType preferType);

    void release();

    int sendPacket(const AVPacket *packet);

    int receiveFrame(AVFrame *frame);

    void flush();

    CodecType getCodecType();

    AVPixelFormat getPixelFormat();

private:

    AVCodecContext *codecCtx;
    AVCodec *codec;
    CodecType codecType = CodecType::UNKNOWN;

    // hw
    AVBufferRef *hwDeviceCtx = nullptr;
    AVPixelFormat hwPixFormat = AV_PIX_FMT_NONE;

    int initHWDecoder(AVCodecContext *ctx, const enum AVHWDeviceType type);

    bool findHWDecoder(AVCodecParameters *params, AVCodecID codecId);

    bool findSWDecoder(AVCodecParameters *params, AVCodecID codecId);

};


#endif //ANDROID_VIDEOPLAYER_FFMPEGDECODER_H
