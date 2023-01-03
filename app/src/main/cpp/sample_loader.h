//
// Created by 祖国瑞 on 2023/1/3.
//

#ifndef ANDROID_VIDEOPLAYER_SAMPLE_LOADER_H
#define ANDROID_VIDEOPLAYER_SAMPLE_LOADER_H

#include <stdlib.h>
#include <stdint.h>
#include "SampleType.h"
#include "SampleLayout.h"
extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

SampleType get_sample_type(AVSampleFormat format);

SampleLayout get_sample_layout(AVSampleFormat format);

int64_t compute_buffer_size(AVSampleFormat format, int numChannel, int numFrame);

void merge_channels(uint8_t **src, uint8_t *dst, int sampleSize, int numChannel, int numFrame);

void separate_channels(uint8_t *src, uint8_t **dst, int sampleSize, int numChannel, int numFrame);

#endif //ANDROID_VIDEOPLAYER_SAMPLE_LOADER_H
