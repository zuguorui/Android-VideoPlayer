//
// Created by incus on 2020-04-15.
//

#ifndef ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
#define ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H

#include <iostream>
#include <stdlib.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "IAudioPlayer.h"
#include "IAudioFrameProvider.h"

class OpenSLESPlayer: public IAudioPlayer{
public:
    OpenSLESPlayer();
    ~OpenSLESPlayer();

    bool create() override;

    void release() override;

    void start() override;

    void stop() override;

    void setAudioFrameProvider(IAudioFrameProvider *provider) override;

    void removeAudioFrameProvider(IAudioFrameProvider *provider) override;
};


#endif //ANDROID_VIDEOPLAYER_OPENSLESPLAYER_H
