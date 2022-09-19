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
    const char* getName();

    bool init(int codecId, AVCodecParameters *params);

    void release();

    DecodingState sendPacket(const AVPacket *packet);

    std::unique_ptr<void> receiveFrame();

private:

    AVCodecContext *codecCtx;
    AVCodec *codec;

};


#endif //ANDROID_VIDEOPLAYER_FFMPEGDECODER_H
