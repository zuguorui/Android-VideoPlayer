//
// Created by 祖国瑞 on 2022/9/22.
//

#ifndef ANDROID_VIDEOPLAYER_UTIL_H
#define ANDROID_VIDEOPLAYER_UTIL_H

extern "C" {
#include "FFmpeg/libavutil/frame.h"
#include "FFmpeg/libavcodec/packet.h"
}

static void av_frame_ptr_deleter(AVFrame *frame) {
    av_frame_free(&frame);
}

static void av_packet_ptr_deleter(AVPacket *packet) {
    av_packet_free(&packet);
}

#endif //ANDROID_VIDEOPLAYER_UTIL_H
