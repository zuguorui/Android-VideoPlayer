//
// Created by 祖国瑞 on 2022/9/22.
//

#ifndef ANDROID_VIDEOPLAYER_UTILS_H
#define ANDROID_VIDEOPLAYER_UTILS_H

#include <chrono>

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

static int64_t getSystemClockCurrentMilliseconds() {
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}

static int64_t getSystemClockCurrentMicroseconds() {
    return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}



#endif //ANDROID_VIDEOPLAYER_UTILS_H
