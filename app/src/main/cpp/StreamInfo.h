//
// Created by 祖国瑞 on 2022/9/24.
//

#ifndef ANDROID_VIDEOPLAYER_STREAMINFO_H
#define ANDROID_VIDEOPLAYER_STREAMINFO_H

#include <stdio.h>
#include <stdlib.h>
#include "CodecType.h"
extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
};

struct StreamInfo {

    int streamIndex = -1;
    AVMediaType type = AVMEDIA_TYPE_UNKNOWN;
    int64_t durationMS = -1;

    CodecType codecType = CodecType::UNKNOWN;

    // audio
    int32_t sampleRate = -1;
    int32_t channels = -1;

    // video
    int32_t width = -1;
    int32_t height = -1;
    float fps = -1;

};


#endif //ANDROID_VIDEOPLAYER_STREAMINFO_H
