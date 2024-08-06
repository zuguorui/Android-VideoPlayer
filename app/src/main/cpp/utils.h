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


static int64_t getSystemClockCurrentMilliseconds() {
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}

static int64_t getSystemClockCurrentMicroseconds() {
    return std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}

static const char* av_get_pix_fmt_name(AVPixelFormat format) {
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV444P:
            return "YUV444P";
        case AVPixelFormat::AV_PIX_FMT_YUV444P10BE:
            return "YUV444P10LE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P10LE:
            return "YUV444P10LE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P12BE:
            return "YUV444P12BE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P12LE:
            return "YUV444P12LE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P14BE:
            return "YUV444P14BE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P14LE:
            return "YUV444P14LE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P16BE:
            return "YUV444P16BE";
        case AVPixelFormat::AV_PIX_FMT_YUV444P16LE:
            return "YUV444P16LE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P:
            return "YUV422P";
        case AVPixelFormat::AV_PIX_FMT_YUV422P10BE:
            return "YUV422P10BE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P10LE:
            return "YUV422P10LE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P12BE:
            return "YUV422P12BE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P12LE:
            return "YUV422P12LE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P14BE:
            return "YUV422P14BE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P14LE:
            return "YUV422P14LE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P16BE:
            return "YUV422P16BE";
        case AVPixelFormat::AV_PIX_FMT_YUV422P16LE:
            return "YUV422P16LE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P:
            return "YUV420P";
        case AVPixelFormat::AV_PIX_FMT_NV12:
            return "NV12";
        case AVPixelFormat::AV_PIX_FMT_NV21:
            return "NV21";
        case AVPixelFormat::AV_PIX_FMT_YUV420P10BE:
            return "YUV420P10BE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P10LE:
            return "YUV420P10LE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P12BE:
            return "YUV420P12BE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P12LE:
            return "YUV420P12LE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P14BE:
            return "YUV420P14BE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P14LE:
            return "YUV420P14LE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P16BE:
            return "YUV420P16BE";
        case AVPixelFormat::AV_PIX_FMT_YUV420P16LE:
            return "YUV420P16LE";
        case AVPixelFormat::AV_PIX_FMT_YUYV422:
            return "YUYV422";
        case AVPixelFormat::AV_PIX_FMT_YVYU422:
            return "YVYU422";
        case AVPixelFormat::AV_PIX_FMT_UYVY422:
            return "UYVY422";
        case AVPixelFormat::AV_PIX_FMT_UYYVYY411:
            return "UYYVYY411";
        case AVPixelFormat::AV_PIX_FMT_RGB24:
            return "RGB24";
        case AVPixelFormat::AV_PIX_FMT_RGB555BE:
            return "RGB555BE";
        case AVPixelFormat::AV_PIX_FMT_RGB555LE:
            return "RGB555LE";
        case AVPixelFormat::AV_PIX_FMT_RGB565BE:
            return "RGB565BE";
        case AVPixelFormat::AV_PIX_FMT_RGB565LE:
            return "RGB565LE";
        case AVPixelFormat::AV_PIX_FMT_BGR24:
            return "BGR24";
        case AVPixelFormat::AV_PIX_FMT_BGR555BE:
            return "BGR555BE";
        case AVPixelFormat::AV_PIX_FMT_BGR555LE:
            return "BGR555LE";
        case AVPixelFormat::AV_PIX_FMT_BGR565BE:
            return "BGR565BE";
        case AVPixelFormat::AV_PIX_FMT_BGR565LE:
            return "BGR565LE";
        default:
            return "unknown" + format;
    }
}

#endif //ANDROID_VIDEOPLAYER_UTILS_H
