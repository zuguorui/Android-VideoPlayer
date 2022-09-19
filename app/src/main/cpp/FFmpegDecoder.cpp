//
// Created by 祖国瑞 on 2022/9/19.
//

#include "FFmpegDecoder.h"
#include "Log.h"

static const char* TAG = "FFmpegDecoder";

using namespace std;

const char* FFmpegDecoder::getName() {
    if (codec == nullptr) {
        return "Not init";
    }
    return codec->name;
}

bool FFmpegDecoder::init(int codecId, AVCodecParameters *params) {
    int ret;
    AVCodecID ffCodecID = AV_CODEC_ID_NONE;
    try {
        ffCodecID = AVCodecID(codecId);
    } catch (...) {
        LOGE(TAG, "failed to convert %d to AVCodecID", codecId);
        return false;
    }
    codec = avcodec_find_decoder(ffCodecID);
    if (!codec) {
        LOGE(TAG, "Can't find decoder for codecID %d", ffCodecID);
        return false;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGE(TAG, "failed to alloc codec context");
        return false;
    }

    if (params != nullptr) {
        ret = avcodec_parameters_to_context(codecCtx, params);
        if (ret < 0) {
            LOGE(TAG, "copy decoder params failed, err = %d", ret);
            return false;
        }
    }

    ret = avcodec_open2(codecCtx, codec, nullptr);
    if (ret < 0) {
        LOGE(TAG, "open codec failed, err = %d", ret);
        return false;
    }

    return true;
}

void FFmpegDecoder::release() {
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
        codec = nullptr;
    }
}

DecodingState FFmpegDecoder::sendPacket(const AVPacket *packet) {
    if (!codecCtx) {
        return DecodingState::NOT_INIT;
    }
    int ret = 0;
    ret = avcodec_send_packet(codecCtx, packet);
    if (ret == AVERROR(EAGAIN)) {
        return DecodingState::AGAIN;
    } else if (ret == AVERROR_EOF) {
        return DecodingState::IS_EOF;
    } else if (ret < 0) {
        LOGE(TAG, "avcodec_send_packet failed, err = %d", ret);
        return DecodingState::UNKNOWN_ERROR;
    }

    return DecodingState::OK;
}

std::unique_ptr<void> FFmpegDecoder::receiveFrame() {
    return NULL;
}
