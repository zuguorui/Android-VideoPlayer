//
// Created by 祖国瑞 on 2023/9/20.
//

#pragma once

#include <stdlib.h>

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
}

#if defined(OS_ANDROID)

#define HW_DEC_COUNT 7

#define HW_CODEC_H264 "h264_mediacodec"
#define HW_CODEC_HEVC "hevc_mediacodec"
#define HW_CODEC_VP8 "vp8_mediacodec"
#define HW_CODEC_VP9 "vp9_mediacodec"
#define HW_CODEC_AV1 "av1_mediacodec"
#define HW_CODEC_MPEG2 "mpeg2_mediacodec"
#define HW_CODEC_MPEG4 "mpeg4_mediacodec"

const static AVCodecID HW_DECODERS[HW_DEC_COUNT] = {
        AVCodecID::AV_CODEC_ID_H264,
        AVCodecID::AV_CODEC_ID_HEVC,
        AVCodecID::AV_CODEC_ID_VP8,
        AVCodecID::AV_CODEC_ID_VP9,
        AVCodecID::AV_CODEC_ID_AV1,
        AVCodecID::AV_CODEC_ID_MPEG2VIDEO,
        AVCodecID::AV_CODEC_ID_MPEG4
};

static const char* HW_CODEC_NAMES[HW_DEC_COUNT] = {
        HW_CODEC_H264,
        HW_CODEC_HEVC,
        HW_CODEC_VP8,
        HW_CODEC_VP9,
        HW_CODEC_AV1,
        HW_CODEC_MPEG2,
        HW_CODEC_MPEG4
};

#elif defined(OS_IOS)

#elif defined(OS_WINDOWS)

#elif defined(OS_LINUX)

#endif

static bool supportHWCodec(AVCodecID codecId) {
    for (AVCodecID id : HW_DECODERS) {
        if (id == codecId) {
            return true;
        }
    }
    return false;
}

static const char* getHWCodecName(AVCodecID codecId) {
    for (int i = 0; i < HW_DEC_COUNT; i++) {
        if (HW_DECODERS[i] == codecId) {
            return HW_CODEC_NAMES[i];
        }
    }
    return nullptr;
}



