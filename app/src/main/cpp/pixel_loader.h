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
#define PIXEL_LAYOUT_SEMI_PLANNER 2

#define PIXEL_TYPE_UNKNOWN -1
#define PIXEL_TYPE_YUV 0
#define PIXEL_TYPE_RGB 1

// get pixel type: yuv or rgb
static int get_pixel_type(AVPixelFormat format);

// get pixel layout: planner, packet or semi-planner
static int get_pixel_layout(AVPixelFormat format);

// get yuv component depth
static int get_yuv_comp_depth(AVPixelFormat format);

// get yuv component pixel count ratio
static bool get_yuv_comp_count_ratio(AVPixelFormat format, int *y2u, int *y2v);

// get yuv component width and height ratio. rgb is always 1:1:1
static bool get_yuv_comp_size_ratio(AVPixelFormat format, int *y2u_width, int *y2u_height, int *y2v_width, int *y2v_height);

// compute needed yuv component buffer size to load a frame with size width*height.
// width, height: a video frame size, in pixels.
// *xSize: component buffer size, in bytes.
static bool compute_yuv_buffer_size(AVPixelFormat format, int64_t width, int64_t height,
                                    int64_t *ySize, int64_t *uSize, int64_t *vSize);


static bool read_yuv_pixel(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                           uint8_t *yBuffer, int *yWidth, int *yHeight,
                           uint8_t *uBuffer, int *uWidth, int *uHeight,
                           uint8_t *vBuffer, int *vWidth, int *vHeight);
// for YUVxxxp
static bool read_yuv_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                             uint8_t *yBuffer, int *yWidth, int *yHeight,
                             uint8_t *uBuffer, int *uWidth, int *uHeight,
                             uint8_t *vBuffer, int *vWidth, int *vHeight);
// for YUYVxxxP
static bool read_yuv_packet(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                            uint8_t *yBuffer, int *yWidth, int *yHeight,
                            uint8_t *uBuffer, int *uWidth, int *uHeight,
                            uint8_t *vBuffer, int *vWidth, int *vHeight);

// for YUVxxxSP，specially NV21 and NV12
static bool read_yuv_semi_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                                  uint8_t *yBuffer, int *yWidth, int *yHeight,
                                  uint8_t *uBuffer, int *uWidth, int *uHeight,
                                  uint8_t *vBuffer, int *vWidth, int *vHeight);





#endif //ANDROID_VIDEOPLAYER_PIXEL_LOADER_H
