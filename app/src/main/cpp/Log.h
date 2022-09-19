//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_LOG_H
#define ANDROID_VIDEOPLAYER_LOG_H

#include <android/log.h>

#define LOGD(TAG, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#endif //ANDROID_VIDEOPLAYER_LOG_H
