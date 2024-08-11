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

struct AudioParameter {
    AVCodecID codecID = AVCodecID::AV_CODEC_ID_NONE;
    AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
    int channels = 0;

    bool check() {
        if (codecID == AVCodecID::AV_CODEC_ID_NONE) {
            return false;
        }
        if (sampleFormat == AVSampleFormat::AV_SAMPLE_FMT_NONE) {
            return false;
        }
        if (channels <= 0) {
            return false;
        }
        return true;
    }
};
