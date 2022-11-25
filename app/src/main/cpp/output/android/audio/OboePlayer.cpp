//
// Created by 祖国瑞 on 2022/9/27.
//

#include "OboePlayer.h"
#include "Log.h"

#define TAG "OboePlayer"

using namespace std;
using namespace oboe;

OboePlayer::OboePlayer(PlayerContext *playerContext): IAudioOutput(playerContext) {

}

OboePlayer::~OboePlayer() {
    stop();
    release();
}

bool OboePlayer::create(int sampleRate, int channels, AVSampleFormat sampleFormat) {
    if (audioStream && this->sampleRate == sampleRate &&
        this->channels == channels && this->sampleFormat == sampleFormat) {
        return true;
    }

    IAudioOutput::create(sampleRate, channels, sampleFormat);

    Result result;
    AudioStreamBuilder builder;
    builder.setDirection(Direction::Output);
    builder.setChannelCount(channels);
    builder.setSampleRate(sampleRate);
    if (sampleFormat == AVSampleFormat::AV_SAMPLE_FMT_S16) {
        builder.setFormat(AudioFormat::I16);
    } else if (sampleFormat == AVSampleFormat::AV_SAMPLE_FMT_S32) {
        builder.setFormat(AudioFormat::I32);
    } else if (sampleFormat == AVSampleFormat::AV_SAMPLE_FMT_FLT) {
        builder.setFormat(AudioFormat::Float);
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

void OboePlayer::write(AudioFrame *audioFrame) {
    if (audioStream && audioFrame) {
        audioStream->write(audioFrame->data, audioFrame->framesPerChannel, -1);
    }
    if (audioFrame) {
        if (playerCtx) {
            playerCtx->recycleAudioFrame(audioFrame);
        } else {
            delete audioFrame;
        }
    }
}

void OboePlayer::write(uint8_t *buffer, int framesPerChannel) {
    if (audioStream) {
        audioStream->write(buffer, framesPerChannel, -1);
    }
}

