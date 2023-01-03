//
// Created by 祖国瑞 on 2023/1/3.
//

#include "sample_loader.h"
#include "Log.h"

#define TAG "sample_loader"

SampleType get_sample_type(AVSampleFormat format) {
    switch (format) {
        case AVSampleFormat::AV_SAMPLE_FMT_U8:
        case AVSampleFormat::AV_SAMPLE_FMT_U8P:
        case AVSampleFormat::AV_SAMPLE_FMT_S16:
        case AVSampleFormat::AV_SAMPLE_FMT_S16P:
        case AVSampleFormat::AV_SAMPLE_FMT_S32:
        case AVSampleFormat::AV_SAMPLE_FMT_S32P:
        case AVSampleFormat::AV_SAMPLE_FMT_S64:
        case AVSampleFormat::AV_SAMPLE_FMT_S64P:
            return SampleType::Int;
        case AVSampleFormat::AV_SAMPLE_FMT_FLT:
        case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
        case AVSampleFormat::AV_SAMPLE_FMT_DBL:
        case AVSampleFormat::AV_SAMPLE_FMT_DBLP:
            return SampleType::Float;
        default:
            return SampleType::None;

    }
}

SampleLayout get_sample_layout(AVSampleFormat format) {
    switch (format) {
        case AVSampleFormat::AV_SAMPLE_FMT_U8:
        case AVSampleFormat::AV_SAMPLE_FMT_S16:
        case AVSampleFormat::AV_SAMPLE_FMT_S32:
        case AVSampleFormat::AV_SAMPLE_FMT_S64:
        case AVSampleFormat::AV_SAMPLE_FMT_FLT:
        case AVSampleFormat::AV_SAMPLE_FMT_DBL:
            return SampleLayout::Packet;
        case AVSampleFormat::AV_SAMPLE_FMT_U8P:
        case AVSampleFormat::AV_SAMPLE_FMT_S16P:
        case AVSampleFormat::AV_SAMPLE_FMT_S32P:
        case AVSampleFormat::AV_SAMPLE_FMT_S64P:
        case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
        case AVSampleFormat::AV_SAMPLE_FMT_DBLP:
            return SampleLayout::Planner;
        default:
            return SampleLayout::None;
    }
}

int64_t compute_buffer_size(AVSampleFormat format, int numChannel, int numFrame) {
    int64_t frameSize = av_get_bytes_per_sample(format);
    return frameSize * numChannel * numFrame;
}

void merge_channels(uint8_t **src, uint8_t *dst, int sampleSize, int numChannel, int numFrame) {
    uint8_t *dstPtr;
    uint8_t *srcPtr;
    for (int index = 0; index < numFrame; index++) {
        for (int channel = 0; channel < numChannel; channel++) {
            dstPtr = dst + (index * numChannel * sampleSize) + (channel * sampleSize);
            srcPtr = src[channel] + index * sampleSize;
            memcpy(dstPtr, srcPtr, sampleSize);
        }
    }
}

void separate_channels(uint8_t *src, uint8_t **dst, int sampleSize, int numChannel, int numFrame) {
    uint8_t *dstPtr;
    uint8_t *srcPtr;
    for (int index = 0; index < numFrame; index++) {
        for (int channel = 0; channel < numChannel; channel++) {
            dstPtr = dst[channel] + index * sampleSize;
            srcPtr = src + (index * numChannel * sampleSize) + (channel * sampleSize);
            memcpy(dstPtr, srcPtr, sampleSize);
        }
    }
}