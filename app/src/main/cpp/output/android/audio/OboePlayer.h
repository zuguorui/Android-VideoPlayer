//
// Created by 祖国瑞 on 2022/9/27.
//

#ifndef ANDROID_VIDEOPLAYER_OBOEPLAYER_H
#define ANDROID_VIDEOPLAYER_OBOEPLAYER_H

#include "oboe/Oboe.h"
#include "AudioFrame.h"
#include "IAudioOutput.h"
#include "PlayerContext.h"


class OboePlayer: public IAudioOutput {
public:
    OboePlayer(PlayerContext *playerContext);

    ~OboePlayer();

    void setSrcFormat(int sampleRate, int channels, AVSampleFormat sampleFormat) override;

    bool create() override;

    void release() override;

    void start() override;

    void stop() override;

    void write(AudioFrame *audioFrame) override;

    void write(uint8_t *buffer, int framesPerChannel) override;


private:

    oboe::AudioStream *audioStream = nullptr;
};


#endif //ANDROID_VIDEOPLAYER_OBOEPLAYER_H
