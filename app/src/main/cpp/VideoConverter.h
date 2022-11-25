//
// Created by 祖国瑞 on 2022/9/29.
//

#ifndef ANDROID_VIDEOPLAYER_VIDEOCONVERTER_H
#define ANDROID_VIDEOPLAYER_VIDEOCONVERTER_H

#include <stdlib.h>

extern "C" {
#include "FFmpeg/libswscale/swscale.h"
#include "FFmpeg/libavformat/avformat.h"
};

class VideoConverter {
public:
    VideoConverter();
    ~VideoConverter();
    int setFormat(int srcWidth, int srcHeight, AVPixelFormat srcFormat, int dstWidth, int dstHeight, AVPixelFormat dstFormat);

    void reset();

    void convert(uint8_t *inputBuffer, uint8_t *outputBuffer);

private:

    SwsContext *swsContext = nullptr;

    AVPixelFormat srcFormat = AVPixelFormat::AV_PIX_FMT_NONE;
    AVPixelFormat dstFormat = AVPixelFormat::AV_PIX_FMT_NONE;

};


#endif //ANDROID_VIDEOPLAYER_VIDEOCONVERTER_H
