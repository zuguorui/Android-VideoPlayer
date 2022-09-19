//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_PLAYER_H
#define ANDROID_VIDEOPLAYER_PLAYER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>

#include "Log.h"
#include "VideoFrame.h"
#include "AudioFrame.h"
#include "PlayerContext.h"
#include "IDecoder.h"
#include "FFmpegDecoder.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
};

class Player {

public:
    Player();
    Player(Player&) = delete;
    Player(Player&&) = delete;
    ~Player();


private:



};


#endif //ANDROID_VIDEOPLAYER_PLAYER_H
