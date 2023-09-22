//
// Created by 祖国瑞 on 2022/9/27.
//

#ifndef ANDROID_VIDEOPLAYER_OBOEPLAYER_H
#define ANDROID_VIDEOPLAYER_OBOEPLAYER_H

#include "oboe/Oboe.h"
#include "AudioFrame.h"
#include "IAudioOutput.h"
#include "PlayerContext.h"
#include "SampleType.h"
#include "SampleLayout.h"


class OboePlayer: public IAudioOutput {
public:
    OboePlayer(PlayerContext *playerContext);

    ~OboePlayer();

    bool create(int sampleRate, int channels, AVSampleFormat sampleFormat) override;

    void release() override;

    void start() override;

    void stop() override;

    bool write(AudioFrame *audioFrame) override;

    void write(uint8_t *buffer, int framesPerChannel) override;


private:

    oboe::AudioStream *audioStream = nullptr;
    SampleLayout sampleLayout = SampleLayout::None;
    int sampleSize = 0;
    SampleType sampleType = SampleType::None;
    uint8_t *packetBuffer = nullptr;
    int64_t packetBufferSize = 0;

    int sampleRate = -1;
    int channels = -1;
    AVSampleFormat sampleFormat = AVSampleFormat::AV_SAMPLE_FMT_NONE;
};


#endif //ANDROID_VIDEOPLAYER_OBOEPLAYER_H
