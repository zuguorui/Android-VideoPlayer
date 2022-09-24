//
// Created by 祖国瑞 on 2022/9/19.
//

#ifndef ANDROID_VIDEOPLAYER_FFMPEGDECODER_H
#define ANDROID_VIDEOPLAYER_FFMPEGDECODER_H

#include "IDecoder.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
};

class FFmpegDecoder: public IDecoder{
public:

    FFmpegDecoder();

    ~FFmpegDecoder();

    const char* getName();

    bool init(AVCodecParameters *params);

    void release();

    CodecState sendPacket(const AVPacket *packet);

    int receiveFrame(AVFrame *frame);

private:

    AVCodecContext *codecCtx;
    AVCodec *codec;

};


#endif //ANDROID_VIDEOPLAYER_FFMPEGDECODER_H
