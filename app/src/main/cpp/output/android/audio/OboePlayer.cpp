//
// Created by 祖国瑞 on 2022/9/27.
//

#include "OboePlayer.h"
#include "Log.h"
#include "sample_loader.h"

#define TAG "OboePlayer"

using namespace std;
using namespace oboe;

OboePlayer::OboePlayer(PlayerContext *playerContext): IAudioOutput(playerContext) {

}

OboePlayer::~OboePlayer() {
    stop();
    release();
}

bool OboePlayer::create() {

    Result result;
    AudioStreamBuilder builder;
    builder.setDirection(Direction::Output);
    builder.setChannelCount(srcChannels);
    builder.setSampleRate(srcSampleRate);

    switch (srcSampleFormat) {
        case AVSampleFormat::AV_SAMPLE_FMT_S16:
        case AVSampleFormat::AV_SAMPLE_FMT_S16P:
            builder.setFormat(AudioFormat::I16);
            break;
        case AVSampleFormat::AV_SAMPLE_FMT_S32:
        case AVSampleFormat::AV_SAMPLE_FMT_S32P:
            builder.setFormat(AudioFormat::I32);
            break;
        case AVSampleFormat::AV_SAMPLE_FMT_FLT:
        case AVSampleFormat::AV_SAMPLE_FMT_FLTP:
            builder.setFormat(AudioFormat::Float);
            break;
        default:
            LOGE(TAG, "unknown sample format: %d", srcSampleFormat);
            return false;
    }



    builder.setAudioApi(AudioApi::AAudio);
    builder.setUsage(Usage::Media);
    builder.setContentType(ContentType::Movie);

    result = builder.openStream(&audioStream);
    if (result != Result::OK) {
        LOGE(TAG, "open audio stream failed, result = %d", result);
        return false;
    }
    return true;
}

void OboePlayer::release() {
    if (audioStream) {
        audioStream->stop();
        audioStream->close();
        delete audioStream;
        audioStream = nullptr;
    }
}

void OboePlayer::start() {
    if (!audioStream) {
        return;
    }
    audioStream->start();
}

void OboePlayer::stop() {
    if (!audioStream) {
        return;
    }
    audioStream->pause();
}

bool OboePlayer::write(AudioFrame *audioFrame) {
    if (audioFrame == nullptr) {
        return false;
    }

    if (sampleLayout == SampleLayout::None) {
        LOGE(TAG, "sampleLayout is none");
        return false;

    }

    if (audioStream == nullptr) {
        LOGE(TAG, "audioStream is none");
        return false;
    }

    if (sampleLayout == SampleLayout::Packet) {

        uint8_t *bufferPtr = audioFrame->avFrame->data[0] + (audioFrame->outputStartIndex * srcChannels * sampleSize);
        audioStream->write(bufferPtr, audioFrame->outputFrameCount, 1000 * 1000 * 1000);

    } else if (sampleLayout == SampleLayout::Planner) {
        int64_t bufferSize = audioFrame->outputFrameCount * srcChannels * sampleSize;
        if (packetBufferSize < bufferSize) {
            if (packetBuffer) {
                free(packetBuffer);
                packetBuffer = nullptr;
            }

            packetBufferSize = bufferSize;
            packetBuffer = (uint8_t *)malloc(packetBufferSize);
        }

        merge_channels(audioFrame->avFrame->data, audioFrame->outputStartIndex, packetBuffer, sampleSize, audioFrame->numChannels, audioFrame->outputFrameCount);

        audioStream->write(packetBuffer, audioFrame->outputFrameCount, 1000 * 1000 * 1000);
    }
    return true;

}

void OboePlayer::write(uint8_t *buffer, int framesPerChannel) {
    if (audioStream) {
        audioStream->write(buffer, framesPerChannel, -1);
    }
}

void OboePlayer::setSrcFormat(int sampleRate, int channels, AVSampleFormat sampleFormat) {
    IAudioOutput::setSrcFormat(sampleRate, channels, sampleFormat);
    sampleSize = av_get_bytes_per_sample(sampleFormat);
    sampleLayout = get_sample_layout(sampleFormat);
    sampleType = get_sample_type(sampleFormat);
}


