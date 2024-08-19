//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_LOG_H
#define ANDROID_VIDEOPLAYER_LOG_H

#include <android/log.h>

extern "C" {
#include "FFmpeg/libavutil/avutil.h"
#include "librtmp/log.h"
}

#define TAG_FFMPEG "FFmpeg"

#define TAG_RTMP "RTMP"

#define LOGV(TAG, ...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGI(TAG, ...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGW(TAG, ...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)



static void ffmpegLogCallback(void *ctx, int level, const char *fmt, va_list args) {
    if(level == AV_LOG_ERROR) {
        __android_log_vprint(ANDROID_LOG_ERROR, TAG_FFMPEG, fmt, args);
    } else if (level == AV_LOG_INFO) {
        __android_log_vprint(ANDROID_LOG_INFO, TAG_FFMPEG, fmt, args);
    } else if (level == AV_LOG_VERBOSE) {
        __android_log_vprint(ANDROID_LOG_VERBOSE, TAG_FFMPEG, fmt, args);
    } else if (level == AV_LOG_WARNING) {
        __android_log_vprint(ANDROID_LOG_WARN, TAG_FFMPEG, fmt, args);
    } else {
        __android_log_vprint(ANDROID_LOG_DEBUG, TAG_FFMPEG, fmt, args);
    }
}

static void rtmpLogCallback(int level, const char *fmt, va_list args) {
    if (level == RTMP_LOGERROR) {
        __android_log_vprint(ANDROID_LOG_ERROR, TAG_RTMP, fmt, args);
    } else if (level == RTMP_LOGINFO) {
        __android_log_vprint(ANDROID_LOG_INFO, TAG_RTMP, fmt, args);
    } else if (level == RTMP_LOGWARNING) {
        __android_log_vprint(ANDROID_LOG_WARN, TAG_RTMP, fmt, args);
    } else {
        __android_log_vprint(ANDROID_LOG_DEBUG, TAG_RTMP, fmt, args);
    }
}



#endif //ANDROID_VIDEOPLAYER_LOG_H
