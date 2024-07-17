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
        LOGE("FFmpeg", fmt, args);
    } else if (level == AV_LOG_INFO) {
        LOGI("FFmpeg", fmt, args);
    } else if (level == AV_LOG_VERBOSE) {
        LOGV("FFmpeg", fmt, args);
    } else if (level == AV_LOG_WARNING) {
        LOGW("FFmpeg", fmt, args);
    } else {
        LOGD("FFmpeg", fmt, args);
    }
}

#endif //ANDROID_VIDEOPLAYER_LOG_H
