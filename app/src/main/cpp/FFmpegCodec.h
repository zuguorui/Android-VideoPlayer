//
// Created by 祖国瑞 on 2022/9/19.
//

#ifndef ANDROID_VIDEOPLAYER_FFMPEGCODEC_H
#define ANDROID_VIDEOPLAYER_FFMPEGCODEC_H

#include "ICodec.h"
#include "platform.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
#include "FFmpeg/libavutil/hwcontext.h"
};

class FFmpegCodec: public ICodec{
public:

    FFmpegCodec();

    ~FFmpegCodec();

    const char* getName() override;

    bool init(AVCodecParameters *params, PreferCodecType preferType, bool isEncoder) override;

    void release() override;

    int sendPacket(const AVPacket *packet) override;

    int receiveFrame(AVFrame *frame) override;

    int sendFrame(const AVFrame *frame) override;

    int receivePacket(AVPacket *packet) override;

    bool isEncoder() override;

    void flush() override;

    CodecType getCodecType() override;

    AVPixelFormat getPixelFormat() override;

private:

    AVCodecContext *codecCtx = nullptr;
    AVCodec *codec = nullptr;
    CodecType codecType = CodecType::UNKNOWN;

    bool _isEncoder = false;

    // hw
    AVBufferRef *hwDeviceCtx = nullptr;
    AVPixelFormat hwPixFormat = AV_PIX_FMT_NONE;

    int initHWCodec(AVCodecContext *ctx, const enum AVHWDeviceType type);

    bool findHWCodec(AVCodecParameters *params, AVCodecID codecId);

    bool findSWCodec(AVCodecParameters *params, AVCodecID codecId);

};


#endif //ANDROID_VIDEOPLAYER_FFMPEGCODEC_H
