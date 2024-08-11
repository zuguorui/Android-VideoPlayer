//
// Created by zu on 2024/8/9.
//

#pragma once

#include <stdlib.h>

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavformat/avio.h"
#include "FFmpeg/libavutil/avutil.h"
#include "FFmpeg/libavcodec/avcodec.h"
}

struct VideoParameter {
    AVCodecID codecID = AVCodecID::AV_CODEC_ID_NONE;
    AVPixelFormat pixelFormat = AVPixelFormat::AV_PIX_FMT_NONE;
    int width = -1;
    int height = -1;
    AVRational fps = {0, 1};
    int profile = -1;
    int level = -1;
    int tier = -1;
    int gop = -1;
    int bitrate = -1;

    bool check() {
        if (codecID == AVCodecID::AV_CODEC_ID_NONE) {
            return false;
        }
        if (pixelFormat == AVPixelFormat::AV_PIX_FMT_NONE) {
            return false;
        }
        if (width <= 0 || height <= 0) {
            return false;
        }
        if (fps.num <= 0 || fps.den <= 0) {
            return false;
        }

        return true;
    }
};
