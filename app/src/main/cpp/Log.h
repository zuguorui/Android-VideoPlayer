//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_LOG_H
#define ANDROID_VIDEOPLAYER_LOG_H

#include <android/log.h>

extern "C" {
#include "FFmpeg/libavutil/avutil.h"
}

#define LOGV(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

static void ffmpegLogCallback(void *ctx, int level, const char *fmt, va_list args)
{
    if(level == AV_LOG_ERROR) {
        __android_log_vprint(ANDROID_LOG_ERROR, "FFmpeg", fmt, args);
    } else if (level == AV_LOG_INFO) {
        __android_log_vprint(ANDROID_LOG_INFO, "FFmpeg", fmt, args);
    } else if (level == AV_LOG_VERBOSE) {
        __android_log_vprint(ANDROID_LOG_VERBOSE, "FFmpeg", fmt, args);
    } else if (level == AV_LOG_WARNING) {
        __android_log_vprint(ANDROID_LOG_WARN, "FFmpeg", fmt, args);
    } else {
        __android_log_vprint(ANDROID_LOG_DEBUG, "FFmpeg", fmt, args);
    }
}

#endif //ANDROID_VIDEOPLAYER_LOG_H
