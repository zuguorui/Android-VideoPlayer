//
// Created by 祖国瑞 on 2022/9/28.
//

#include "Factory.h"

IDecoder *findHWDecoder(AVCodecParameters *params) {
    return nullptr;
}

IDecoder *findSWDecoder(AVCodecParameters *params) {
    FFmpegDecoder *decoder = new FFmpegDecoder();
    if (!decoder->init(params)) {
        delete decoder;
        decoder = nullptr;
    }
    return decoder;
}

IAudioOutput *getAudioOutput(PlayerContext *playerContext) {
#if defined OS_ANDROID
    // android audio output
    return new OboePlayer(playerContext);
#elif defined OS_IOS
    // TODO: add ios audio output
    return nullptr;
#elif defined OS_WINDOWS
    // TODO: add windows audio output
    return nullptr;
#elif defined OS_LINUX
    // TODO: add linux audio output
    return nullptr;
#else
    return nullptr;
#endif
}

IVideoOutput *getVideoOutput(PlayerContext *playerContext) {
#if defined OS_ANDROID
    return new OpenGLESPlayer2(playerContext);
#elif defined OS_IOS
    // TODO: add ios audio output
    return nullptr;
#elif defined OS_WINDOWS
    // TODO: add windows audio output
    return nullptr;
#elif defined OS_LINUX
    // TODO: add linux audio output
    return nullptr;
#else
    return nullptr;
#endif
}