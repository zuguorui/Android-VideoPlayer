//
// Created by 祖国瑞 on 2022/9/29.
//

#include "VideoConverter.h"
#include "Log.h"

#define TAG "VideoConverter"

VideoConverter::VideoConverter() {

}

VideoConverter::~VideoConverter() {
    reset();
}

int VideoConverter::setFormat(int srcWidth, int srcHeight, AVPixelFormat srcFormat, int dstWidth,
                              int dstHeight, AVPixelFormat dstFormat) {
    reset();
    this->srcFormat = srcFormat;
    this->dstFormat = dstFormat;

    swsContext = sws_getContext(srcWidth, srcHeight, srcFormat, dstWidth, dstHeight, dstFormat, 0, nullptr,
                                nullptr, nullptr);

    if (!swsContext) {
        LOGE(TAG, "getSwsContext failed");
        return -1;
    }

    return 0;
}

void VideoConverter::convert(uint8_t *inputBuffer, uint8_t *outputBuffer) {
    if (!swsContext) {
        return;
    }
    sws_scale(swsContext, (const uint8_t * const *)inputBuffer, )
}