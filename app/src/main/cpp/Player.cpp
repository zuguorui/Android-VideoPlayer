//
// Created by 祖国瑞 on 2022/9/7.
//

#include "Player.h"
#include "Constants.h"
#include "Util.h"
#include "Log.h"

#define TAG "Player"

using namespace std;


Player::Player() {
    av_log_set_callback(ffmpegLogCallback);

}

Player::~Player() {

}

bool Player::initAudioOutput(int32_t sampleRate, int32_t channels) {
    return true;
}

bool Player::initVideoOutput(void *window) {
    return true;
}

void Player::release() {
    if (videoDecoder) {
        delete videoDecoder;
        videoDecoder = nullptr;
    }

    if (audioDecoder) {
        delete audioDecoder;
        audioDecoder = nullptr;
    }

    audioStreamMap.clear();
    videoStreamMap.clear();

    if (formatCtx) {
        avformat_free_context(formatCtx);
        formatCtx = nullptr;
    }

    audioStreamIndex = -1;
    videoStreamIndex = -1;
}

bool Player::openFile(string pathStr) {
    release();
    this->filePath = pathStr;

    int ret = 0;

    formatCtx = avformat_alloc_context();
    ret = avformat_open_input(&formatCtx, pathStr.c_str(), nullptr, nullptr);
    if (ret < 0) {
        LOGE(TAG, "avformat_open_input failed, err = %d", ret);
        return false;
    }

    ret = avformat_find_stream_info(formatCtx, nullptr);
    if (ret < 0) {
        LOGE(TAG, "avformat_find_stream_info failed, err = %d", ret);
        return false;
    }

    audioStreamMap.clear();
    videoStreamMap.clear();

    for (int i = 0; i < formatCtx->nb_streams; i++) {
        AVStream *stream = formatCtx->streams[i];
        AVMediaType type = stream->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) {
            StreamInfo trackInfo;
            trackInfo.streamIndex = i;
            trackInfo.type = type;
            trackInfo.durationMS = (int64_t)(stream->duration * av_q2d(stream->time_base) * 1000);
            trackInfo.channels = stream->codecpar->channels;
            trackInfo.sampleRate = stream->codecpar->sample_rate;
            audioStreamMap[i] = trackInfo;
        } else if (type == AVMEDIA_TYPE_VIDEO) {
            StreamInfo trackInfo;
            trackInfo.streamIndex = i;
            trackInfo.type = type;
            trackInfo.durationMS = (int64_t)(stream->duration * av_q2d(stream->time_base) * 1000);
            trackInfo.width = stream->codecpar->width;
            trackInfo.height = stream->codecpar->height;
            trackInfo.fps = (float) av_q2d(stream->avg_frame_rate);
            videoStreamMap[i] = trackInfo;
        }
    }

    // find a audio stream which can be decoded.
    audioStreamIndex = -1;
    findAvailableStreamAndDecoder(audioStreamMap, &audioDecoder, &audioStreamIndex);

    // find a video stream which can be decoded
    videoStreamIndex = -1;
    findAvailableStreamAndDecoder(videoStreamMap, &videoDecoder, &videoStreamIndex);

    if (audioStreamIndex == -1 && videoStreamIndex == -1) {
        LOGE(TAG, "neither audio stream nor video stream found");
        return false;
    }

    if (audioStreamIndex != -1) {
        LOGD(TAG, "select audio index = %d, detail:", audioStreamIndex);
        av_dump_format(formatCtx, audioStreamIndex, nullptr, 0);
    }

    if (videoStreamIndex != -1) {
        LOGD(TAG, "select video index = %d, detail:", videoStreamIndex);
        av_dump_format(formatCtx, videoStreamIndex, nullptr, 0);
    }

    return true;
}

void Player::findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, IDecoder **decoder,
                                           int *streamIndex) {
    if (!formatCtx) {
        return;
    }
    for (auto it = streams.begin(); it != streams.end(); it++) {
        AVStream *stream = formatCtx->streams[it->first];
        *decoder = findHWDecoder(stream->codecpar);
        if (!(*decoder)) {
            LOGD(TAG, "finding HW decoder for stream %d failed, then find SW decoder", it->first);
            *decoder = findSWDecoder(stream->codecpar);
        } else {
            it->second.codecType = CodecType::HW;
        }

        if (!(*decoder)) {
            LOGE(TAG, "finding SW decoder for stream %d failed", it->first);
            it->second.codecType = CodecType::NOT_SUPPORTED;
        }

        if (*decoder) {
            *streamIndex = it->first;
            break;
        }
    }
}

IDecoder *Player::findHWDecoder(AVCodecParameters *params) {
    return nullptr;
}

IDecoder *Player::findSWDecoder(AVCodecParameters *params) {
    FFmpegDecoder *decoder = new FFmpegDecoder();
    if (!decoder->init(params)) {
        delete decoder;
        decoder = nullptr;
    }
    return decoder;
}

void Player::readPacketCallback(void *context) {
    ((Player *)context)->readPacketLoop();
}

void Player::readPacketLoop() {
    if (!formatCtx) {
        LOGE(TAG, "no format context");
        return;
    }
    int ret;
    bool pushSuccess = false;
    while (!stopFlag) {
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            LOGE(TAG, "av_packet_alloc failed");
            return;
        }

        ret = av_read_frame(formatCtx, packet);

        if (ret == AVERROR_EOF) {
            av_packet_free(&packet);
            packet = nullptr;
        } else if (ret < 0) {
            LOGE(TAG, "av_read_frame failed");
            av_packet_free(&packet);
            packet = nullptr;
            return;
        }

        if (packet->stream_index == audioStreamIndex) {
            pushSuccess = audioPacketQueue.push(packet);
            if (!pushSuccess) {
                audioPacketQueue.forcePush(packet);
            }
        } else if (packet->stream_index == videoStreamIndex) {
            pushSuccess = videoPacketQueue.push(packet);
            if (!pushSuccess) {
                videoPacketQueue.forcePush(packet);
            }
        } else {
            av_packet_unref(packet);
            av_packet_free(&packet);
        }
    }
}

void Player::decodeAudioCallback(void *context) {
    ((Player *)context)->decodeAudioLoop();
}

void Player::decodeAudioLoop() {
    if (!formatCtx) {
        return;
    }
    if (!audioDecoder) {
        LOGE(TAG, "audio decoder is null");
        return;
    }
    int ret;
    optional<AVPacket *> packetOpt;
    AVPacket *packet = nullptr;
    AVFrame *frame = av_frame_alloc();
    while (!stopFlag) {
        packetOpt = audioPacketQueue.pop();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "packetOpt has no value");
            break;
        }
        packet = packetOpt.value();
        if (packet == nullptr) {
            LOGD(TAG, "audio stream meets eof");
        }
        ret = audioDecoder->sendPacket(packet);
        if (ret < 0) {
            LOGE(TAG, "audio decoder send packet failed, err = %d", ret);
            break;
        }

        while (true) {
            ret = audioDecoder->receiveFrame(frame);
            if (ret < 0) {
                break;
            }
            // todo: convert AVFrame to AudioFrame

            av_frame_unref(frame);
        }

        if (ret == AVERROR(EAGAIN)) {
            LOGD(TAG, "audio stream again");
            continue;
        } else if (ret == AVERROR_EOF) {
            LOGD(TAG, "audio stream meets eof");
            break;
        } else {
            LOGE(TAG, "audio decoder error: %d", ret);
            break;
        }
    }

    if (packet) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }

    if (frame) {
        av_frame_unref(frame);
        av_frame_free(&frame);
        frame = nullptr;
    }

    LOGD(TAG, "audio decode loop finish");

}

void Player::decodeVideoCallback(void *context) {
    ((Player *)context)->decodeVideoLoop();
}

void Player::decodeVideoLoop() {

}



void Player::syncCallback(void *context) {
    ((Player *)context)->syncLoop();
}

void Player::syncLoop() {

}


