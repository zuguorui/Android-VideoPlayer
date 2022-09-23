//
// Created by 祖国瑞 on 2022/9/7.
//

#include "Player.h"
#include "Constants.h"
#include "Util.h"

using namespace std;


Player::Player() {
    frame = av_frame_alloc();
    packet = av_packet_alloc();
}

Player::~Player() {
    if (frame) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (packet) {
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (videoDecoder) {
        delete videoDecoder;
        videoDecoder = nullptr;
    }

    if (audioDecoder) {
        delete audioDecoder;
        audioDecoder = nullptr;
    }
}

