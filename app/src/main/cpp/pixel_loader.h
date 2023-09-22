//
// Created by 祖国瑞 on 2022/12/11.
//

#ifndef ANDROID_VIDEOPLAYER_PIXEL_LOADER_H
#define ANDROID_VIDEOPLAYER_PIXEL_LOADER_H

#include <stdlib.h>
#include <iostream>
#include "Log.h"
#include "PixelType.h"
#include "PixelLayout.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

// I can't setFormat 16bit texture for yuv on Android, so I convert yuv pixels more than 8bit depth to
// float32, this will cost a lot. If you can make 16bit texture work, you can undefine
// LOAD_MULTI_BYTES_YUV_AS_FLOAT, then yuv pixels with more than 8bit depths will be load as
// uint16.
// TODO: Batter to make 16bit texture work to reduce pixel convert cost.

#ifdef OS_ANDROID
#define LOAD_MULTI_BYTES_YUV_AS_FLOAT
#endif


// get pixel type: yuv or rgb
PixelType get_pixel_type(AVPixelFormat format);

// get pixel layout: planner, packet or semi-planner
PixelLayout get_pixel_layout(AVPixelFormat format);

// get yuv raw component depth, in bits.
int get_yuv_comp_depth(AVPixelFormat format);

// get yuv component pixel count ratio
bool get_yuv_comp_count_ratio(AVPixelFormat format, int *y2u, int *y2v);

// get yuv component width and height ratio. rgb is always 1:1:1
bool get_yuv_comp_size_ratio(AVPixelFormat format, int *y2u_width, int *y2u_height, int *y2v_width, int *y2v_height);

// compute needed yuv component buffer size to load a frame with size width*height.
// width, height: a video frame size, in pixels.
// *xSize: component buffer size, in bytes.
// format: if pixel depth <= 8, one pixel in per component uses 1 byte.
//      if pixel depth <= 16, there two conditions:
//          1. if define LOAD_MULTI_BYTES_YUV_AS_FLOAT, pixels will be convert to float32.
//              so one pixel per component uses 4 bytes.
//          2. if not define LOAD_MULTI_BYTES_YUV_AS_FLOAT, pixels will be convert to uint16
//              one pixel per component uses 2 bytes.
//      pixel depth > 16 is not supported.
bool compute_yuv_buffer_size(AVPixelFormat format, int64_t width, int64_t height,
                                    int64_t *ySize, int64_t *uSize, int64_t *vSize);

// read YUV pixel.
// frame: src video frame
// format: if pixel depth <= 8, pixels will be load as uint8.
//      if pixel depth <= 16, there two conditions:
//          1. if define LOAD_MULTI_BYTES_YUV_AS_FLOAT, pixels will be convert to float32.
//          2. if not define LOAD_MULTI_BYTES_YUV_AS_FLOAT, pixels will be convert to uint16
//      pixel depth > 16 is not supported.
// xBuffer: output buffer, this buffer is created by user. You can get buffer size by
//      'compute_yuv_buffer_size'
// *xWidth, *xHeight: loaded size for components.
// return: success or not
// ATTENTION: some formats like
bool read_yuv_pixel(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                           uint8_t *yBuffer, int *yWidth, int *yHeight,
                           uint8_t *uBuffer, int *uWidth, int *uHeight,
                           uint8_t *vBuffer, int *vWidth, int *vHeight);
// for YUVxxxp
bool read_yuv_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                             uint8_t *yBuffer, int *yWidth, int *yHeight,
                             uint8_t *uBuffer, int *uWidth, int *uHeight,
                             uint8_t *vBuffer, int *vWidth, int *vHeight);
// for YUYVxxxP
bool read_yuv_packet(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                            uint8_t *yBuffer, int *yWidth, int *yHeight,
                            uint8_t *uBuffer, int *uWidth, int *uHeight,
                            uint8_t *vBuffer, int *vWidth, int *vHeight);

// for YUVxxxSP，specially NV21 and NV12
bool read_yuv_semi_planner(AVFrame *frame, AVPixelFormat format, int64_t width, int64_t height,
                                  uint8_t *yBuffer, int *yWidth, int *yHeight,
                                  uint8_t *uBuffer, int *uWidth, int *uHeight,
                                  uint8_t *vBuffer, int *vWidth, int *vHeight);





#endif //ANDROID_VIDEOPLAYER_PIXEL_LOADER_H
