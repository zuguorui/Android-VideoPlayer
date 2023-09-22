//
// Created by 祖国瑞 on 2022/9/28.
//

/**
 * This file contains some methods to setFormat components which associated with platforms.
 * */

#ifndef ANDROID_VIDEOPLAYER_FACTORY_H
#define ANDROID_VIDEOPLAYER_FACTORY_H

#include "PlayerContext.h"
#include "IDecoder.h"
#include "FFmpegDecoder.h"
#include "output/IAudioOutput.h"
#include "output/IVideoOutput.h"

#if defined(OS_ANDROID)
#include "output/android/audio/OboePlayer.h"
#include "output/android/video/OpenGLESPlayer2.h"
#elif defined(OS_IOS)

#elif defined(OS_WINDOWS)

#elif defined(OS_LINUX)

#endif


extern "C" {
#include "FFmpeg/libavformat/avformat.h"
}

IAudioOutput *getAudioOutput(PlayerContext *playerContext);
IVideoOutput *getVideoOutput(PlayerContext *playerContext);


#endif //ANDROID_VIDEOPLAYER_FACTORY_H
