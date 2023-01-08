//
// Created by 祖国瑞 on 2023/1/8.
//

#ifndef ANDROID_VIDEOPLAYER_PACKETWRAPPER_H
#define ANDROID_VIDEOPLAYER_PACKETWRAPPER_H

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
#include "FFmpeg/libavutil/avutil.h"
}

struct PacketWrapper {
    AVPacket *avPacket = nullptr;
    int32_t flags = 0;

    PacketWrapper() {
        flags = 0;
    }

    PacketWrapper(AVPacket *packet): PacketWrapper() {
        this->avPacket = packet;
    }

    PacketWrapper(const PacketWrapper &packetWrapper) = delete;

    PacketWrapper(PacketWrapper &&src) {
        this->avPacket = src.avPacket;
        this->flags = src.flags;
        src.avPacket = nullptr;
        src.flags = 0;
    }

    void setParams(AVPacket *packet) {
        this->avPacket = packet;
    }

    void reset() {
        if (avPacket != nullptr) {
            av_packet_free(&avPacket);
            avPacket = nullptr;
        }

        flags = 0;
    }
};

#endif //ANDROID_VIDEOPLAYER_PACKETWRAPPER_H
