//
// Created by 祖国瑞 on 2022/12/11.
//

#ifndef ANDROID_VIDEOPLAYER_PIXEL_LOADER_H
#define ANDROID_VIDEOPLAYER_PIXEL_LOADER_H

#include <stdlib.h>
#include <iostream>
#include "Log.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}


#define PIXEL_LAYOUT_UNKNOWN -1
#define PIXEL_LAYOUT_PLANNER 0
#define PIXEL_LAYOUT_PACKET 1

#define PIXEL_TYPE_UNKNOWN -1
#define PIXEL_TYPE_YUV 0
#define PIXEL_TYPE_RGB 1

static int get_pixel_type(AVPixelFormat format);

static int get_pixel_layout(AVPixelFormat format);

static int get_yuv_comp_depth(AVPixelFormat format);

static bool compute_yuv_count_ratio(AVPixelFormat format, int *y2u, int *y2v);

static bool compute_yuv_size_ratio(AVPixelFormat format, int *y2u_width, int *y2u_height, int *y2v_width, int *y2v_height);

static bool compute_yuv_buffer_size(AVPixelFormat format, int64_t width, int64_t height,
                                    int64_t *ySize, int64_t *uSize, int64_t *vSize);

static bool read_yuv_pixel(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                           void *yBuffer, int *yWidth, int *yHeight,
                           void *uBuffer, int *uWidth, int *uHeight,
                           void *vBuffer, int *vWidth, int *vHeight);

static bool read_yuv_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                             void *yBuffer, int *yWidth, int *yHeight,
                             void *uBuffer, int *uWidth, int *uHeight,
                             void *vBuffer, int *vWidth, int *vHeight);

static bool read_yuv_packet(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                            void *yBuffer, int *yWidth, int *yHeight,
                            void *uBuffer, int *uWidth, int *uHeight,
                            void *vBuffer, int *vWidth, int *vHeight);



#endif //ANDROID_VIDEOPLAYER_PIXEL_LOADER_H
