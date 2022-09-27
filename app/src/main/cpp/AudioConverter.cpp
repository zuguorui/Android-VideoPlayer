//
// Created by 祖国瑞 on 2022/9/27.
//

#include "AudioConverter.h"
#include "Log.h"

#define TAG "AudioConverter"

AudioConverter::AudioConverter() {
}

AudioConverter::~AudioConverter() {
    reset();
}

int AudioConverter::setFormat(int srcSampleRate, int srcChannels, AVSampleFormat srcSampleFormat,
                               int dstSampleRate, int dstChannels, AVSampleFormat dstSampleFormat) {

    reset();

    LOGD(TAG, "sacSampleRate = %d, srcChannels = %d, srcFormat = %d, "
              "dstSampleRate = %d, dstChannels = %d, dstFormat = %d",
              srcSampleRate, srcChannels, srcSampleFormat,
              dstSampleRate, dstChannels, dstSampleFormat);

    this->srcSampleRate = srcSampleRate;
    this->srcChannels = srcChannels;
    this->srcSampleFormat = srcSampleFormat;
    this->dstSampleRate = dstSampleRate;
    this->dstChannels = dstChannels;
    this->dstSampleFormat = dstSampleFormat;

    srcSampleSize = av_get_bytes_per_sample(srcSampleFormat);
    dstSampleSize = av_get_bytes_per_sample(dstSampleFormat);

    int64_t srcChannelLayout = av_get_default_channel_layout(srcChannels);
    int64_t dstChannelLayout = av_get_default_channel_layout(dstChannels);

    swrContext = swr_alloc_set_opts(nullptr, dstChannelLayout, dstSampleFormat, dstSampleRate, srcChannelLayout, srcSampleFormat, srcSampleRate, 0,
                                    nullptr);

    int ret = swr_init(swrContext);

    if (ret < 0) {
        reset();
    }

    return ret;
}

size_t AudioConverter::convert(uint8_t *inputBuffer, size_t inputSizeInBytes, uint8_t *outputBuffer,
                               size_t outputBufferSizeInBytes) {
    if (!swrContext) {
        return -1;
    }

    int outputBufferSizeInSamples = outputBufferSizeInBytes / dstChannels / dstSampleSize;
    int inputSizeInSamples = inputSizeInBytes / srcChannels / srcSampleSize;

    size_t outputSamples = swr_convert(swrContext, &outputBuffer, outputBufferSizeInSamples, (const uint8_t **)&inputBuffer, inputSizeInSamples);

    return outputSamples;
}

void AudioConverter::reset() {
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = nullptr;
    }
}