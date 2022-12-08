//
// Created by 祖国瑞 on 2022/9/7.
//

#include "Player.h"
#include "Constants.h"
#include "Util.h"
#include "Log.h"
#include <list>

#define TAG "Player"

using namespace std;


Player::Player() {
    av_log_set_callback(ffmpegLogCallback);

}

Player::~Player() {
    release();
}

void Player::setWindow(ANativeWindow *window) {
    nativeWindow = window;
}

void Player::release() {
    pause();
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

    if (unPlayedVideoFrame) {
        playerContext.recycleVideoFrame(unPlayedVideoFrame);
        unPlayedVideoFrame = nullptr;
    }

    if (unPlayedAudioFrame) {
        playerContext.recycleAudioFrame(unPlayedAudioFrame);
        unPlayedAudioFrame = nullptr;
    }

    audioStreamIndex = -1;
    videoStreamIndex = -1;

    if (audioOutput != nullptr) {
        audioOutput->release();
        delete(audioOutput);
        audioOutput = nullptr;
    }

    if (videoOutput != nullptr) {
        videoOutput->release();
        delete(videoOutput);
        videoOutput = nullptr;
    }
}

bool Player::openFile(string pathStr) {
    release();
    if (nativeWindow == nullptr) {
        LOGE(TAG, "nativeWindow not set");
        return false;
    }
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
        audioOutput = getAudioOutput(&playerContext);
        if (!audioOutput) {
            LOGE(TAG, "getAudioOutput returns null");
            release();
            return false;
        }
        AVCodecParameters *params = formatCtx->streams[audioStreamIndex]->codecpar;
        audioOutput->setSrcFormat(params->sample_rate, params->channels, static_cast<AVSampleFormat>(params->format));
        if (!audioOutput->create()) {
            LOGE(TAG, "audio output create failed, sampleRate = %d, channels = %d, sampleFormat = %d", params->sample_rate, params->channels, params->format);
            release();
            return false;
        }

    }

    if (videoStreamIndex != -1) {
        LOGD(TAG, "select video index = %d, detail:", videoStreamIndex);
        av_dump_format(formatCtx, videoStreamIndex, nullptr, 0);
        videoOutput = getVideoOutput(&playerContext);
        if (!videoOutput) {
            LOGE(TAG, "getVideoOutput returns null");
            release();
            return false;
        }
        AVCodecParameters *params = formatCtx->streams[videoStreamIndex]->codecpar;
        params->bits_per_raw_sample
        videoOutput->setSrcFormat(static_cast<AVPixelFormat>(params->format));
        if (!videoOutput) {
            LOGE(TAG, "video output create failed, pixelFormat = %d", params->format);
            release();
            return false;
        }

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


void Player::readStreamCallback(void *context) {
    ((Player *)context)->readStreamLoop();
}

void Player::readStreamLoop() {
    if (!formatCtx) {
        LOGE(TAG, "no format context");
        return;
    }
    int ret;
    bool pushSuccess = false;
    list<AVPacket *> ml;
    while (!stopReadFlag) {
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            LOGE(TAG, "av_packet_alloc failed");
            return;
        }

        if (seekReq) {
            int64_t pts = (int64_t)(seekPtsMS / av_q2d(AV_TIME_BASE_Q));
            int streamIndex = -1;
            if (audioStreamIndex >= 0) {
                pts = (int64_t)(seekPtsMS / av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
                streamIndex = audioStreamIndex;
            } else if (videoStreamIndex >= 0) {
                pts = (int64_t)(seekPtsMS / av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
                streamIndex = videoStreamIndex;
            }

            av_seek_frame(formatCtx, streamIndex, pts, AVSEEK_FLAG_BACKWARD);
            audioPacketQueue.clear();
            videoPacketQueue.clear();
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
            ml.push_back(packet);
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
    AVFrame *frame = nullptr;
    AudioFrame *audioFrame = nullptr;
    while (!stopDecodeAudioFlag) {
        packetOpt = audioPacketQueue.pop();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "audio packetOpt has no value");
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
            frame = av_frame_alloc();
            ret = audioDecoder->receiveFrame(frame);
            if (ret < 0) {
                av_frame_unref(frame);
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }
            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->setParams(frame, audioStreamMap[audioStreamIndex].sampleFormat, formatCtx->streams[audioStreamIndex]->time_base);
            if (!audioFrameQueue.push(audioFrame)) {
                audioFrameQueue.push(audioFrame, false);
            }
            // DON'T delete AVFrame here, it will be carried to output by AudioFrame
            audioFrame = nullptr;
            frame = nullptr;
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

    if (audioFrame) {
        audioFrameQueue.push(audioFrame, false);
        audioFrame = nullptr;
    }

    LOGD(TAG, "audio decode loop finish");

}

void Player::decodeVideoCallback(void *context) {
    ((Player *)context)->decodeVideoLoop();
}

void Player::decodeVideoLoop() {
    if (!formatCtx) {
        return;
    }
    if (!videoDecoder) {
        LOGE(TAG, "video decoder is null");
        return;
    }
    int ret;
    optional<AVPacket *> packetOpt;
    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;
    VideoFrame *videoFrame = nullptr;
    while(!stopDecodeVideoFlag) {
        packetOpt = videoPacketQueue.pop();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "video packetOpt has no value");
            break;
        }
        packet = packetOpt.value();
        if (packet == nullptr) {
            LOGD(TAG, "video stream meets eof");
        }
        ret = videoDecoder->sendPacket(packet);
        if (ret < 0) {
            LOGE(TAG, "video decoder send packet failed, err = %d", ret);
            break;
        }

        while (true) {
            frame = av_frame_alloc();
            ret = videoDecoder->receiveFrame(frame);
            if (ret < 0) {
                av_frame_unref(frame);
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }
            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->setParams(frame, videoStreamMap[videoStreamIndex].pixelFormat, formatCtx->streams[videoStreamIndex]->time_base);
            if (!videoFrameQueue.push(videoFrame)) {
                videoFrameQueue.push(videoFrame, false);
            }
            // DON'T delete AVFrame, it will be carried to output by VideoFrame.
            videoFrame = nullptr;
            frame = nullptr;

        }

        if (ret == AVERROR(EAGAIN)) {
            LOGD(TAG, "video stream again");
            continue;
        } else if (ret == AVERROR_EOF) {
            LOGD(TAG, "video stream meets eof");
            break;
        } else {
            LOGE(TAG, "video decoder error: %d", ret);
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

    if (videoFrame) {
        videoFrameQueue.push(videoFrame, false);
        videoFrame = nullptr;
    }

    LOGD(TAG, "video decode loop finish");

}



void Player::syncCallback(void *context) {
    ((Player *)context)->syncLoop();
}

void Player::syncLoop() {
    chrono::system_clock::time_point lastAudioWriteTime;
    AudioFrame *audioFrame = unPlayedAudioFrame;
    unPlayedAudioFrame = nullptr;
    VideoFrame *videoFrame = unPlayedVideoFrame;
    unPlayedVideoFrame = nullptr;
    bool firstLoop = true;
    while (!stopSyncFlag) {
        if (audioFrame == nullptr) {
            optional<AudioFrame *> frameOpt = audioFrameQueue.pop();
            if (frameOpt.has_value()) {
                audioFrame = frameOpt.value();
            } else {
                break;
            }
        }

        if (videoFrame == nullptr) {
            optional<VideoFrame *> frameOpt = videoFrameQueue.pop();
            if (frameOpt.has_value()) {
                videoFrame = frameOpt.value();
            } else {
                break;
            }
        }

        if (videoFrame->pts < audioFrame->pts) {
            if (firstLoop) {
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                lastAudioWriteTime = chrono::system_clock::now();
                lastAudioPts = audioFrame->pts;
                audioOutput->write(audioFrame);
                audioFrame = nullptr;
                firstLoop = false;
            } else {
                if (videoFrame->pts < lastAudioPts) {
                    playerContext.recycleVideoFrame(videoFrame);
                    videoFrame = nullptr;
                    continue;
                } else {
                    chrono::system_clock::time_point now = chrono::system_clock::now();
                    int64_t duration = chrono::duration_cast<chrono::milliseconds>(now - lastAudioWriteTime).count();
                    if (duration > videoFrame->pts - lastAudioPts) {
                        int64_t waitMS = duration - (videoFrame->pts - lastAudioPts);
                        this_thread::sleep_for(chrono::milliseconds(waitMS));
                    }
                    lastVideoPts = videoFrame->pts;
                    videoOutput->write(videoFrame);
                    videoFrame = nullptr;
                }

            }
        } else {
            lastAudioWriteTime = chrono::system_clock::now();
            lastAudioPts = audioFrame->pts;
            audioOutput->write(audioFrame);
            audioFrame = nullptr;
            firstLoop = false;
        }
    }

    if (audioFrame) {
        unPlayedAudioFrame = audioFrame;
    }

    if (videoFrame) {
        unPlayedVideoFrame = videoFrame;
    }
}

void Player::startReadStreamThread() {
    unique_lock<mutex> mLock(readStreamMu);
    if (readStreamThread != nullptr) {
        return;
    }
    audioPacketQueue.setBlockingPush(true);
    videoPacketQueue.setBlockingPush(true);
    stopReadFlag = false;
    readStreamThread = new thread(readStreamCallback, this);

}


void Player::stopReadStreamThread() {
    unique_lock<mutex> mLock(readStreamMu);
    if (readStreamThread == nullptr) {
        return;
    }
    stopReadFlag = true;
    audioPacketQueue.setBlockingPush(false);
    videoPacketQueue.setBlockingPush(false);

    if (readStreamThread->joinable()) {
        readStreamThread->join();
    }

    delete(readStreamThread);
    readStreamThread = nullptr;

    audioPacketQueue.setBlockingPush(true);
    videoPacketQueue.setBlockingPush(true);
    stopReadFlag = false;
}

void Player::startDecodeAudioThread() {
    unique_lock<mutex> mLock(decodeAudioMu);
    if (decodeAudioThread != nullptr) {
        return;
    }

    stopDecodeAudioFlag = false;
    audioPacketQueue.setBlockingPop(true);
    audioFrameQueue.setBlockingPush(true);

    decodeAudioThread = new thread(decodeAudioCallback, this);
}

void Player::stopDecodeAudioThread() {
    unique_lock<mutex> mLock(decodeAudioMu);
    if (decodeAudioThread == nullptr) {
        return;
    }
    stopDecodeAudioFlag = true;
    audioPacketQueue.setBlockingPop(false);
    audioFrameQueue.setBlockingPush(false);

    if (decodeAudioThread->joinable()) {
        decodeAudioThread->join();
    }

    delete(decodeAudioThread);
    decodeAudioThread = nullptr;

    stopDecodeAudioFlag = false;
    audioPacketQueue.setBlockingPop(true);
    audioFrameQueue.setBlockingPush(true);
}

void Player::startDecodeVideoThread() {
    unique_lock<mutex> mLock(decodeVideoMu);
    if (decodeVideoThread != nullptr) {
        return;
    }

    stopDecodeVideoFlag = false;
    videoPacketQueue.setBlockingPop(true);
    videoFrameQueue.setBlockingPush(true);

    decodeVideoThread = new thread(decodeVideoCallback, this);

}

void Player::stopDecodeVideoThread() {
    unique_lock<mutex> mLock(decodeVideoMu);
    if (decodeVideoThread == nullptr) {
        return;
    }

    stopDecodeVideoFlag = true;
    videoPacketQueue.setBlockingPop(false);
    videoFrameQueue.setBlockingPush(false);

    if (decodeVideoThread->joinable()) {
        decodeVideoThread->join();
    }

    delete(decodeVideoThread);
    decodeVideoThread = nullptr;

    stopDecodeVideoFlag = false;
    videoPacketQueue.setBlockingPop(true);
    videoFrameQueue.setBlockingPush(true);
}

void Player::startSyncThread() {
    unique_lock<mutex> mLock(syncMu);
    if (syncThread != nullptr) {
        return;
    }

    stopSyncFlag = false;
    audioFrameQueue.setBlockingPop(true);
    videoFrameQueue.setBlockingPop(true);

    syncThread = new thread(syncCallback, this);
}

void Player::stopSyncThread() {
    unique_lock<mutex> mLock(syncMu);
    if (syncThread == nullptr) {
        return;
    }
    stopSyncFlag = true;
    audioFrameQueue.setBlockingPop(false);
    videoFrameQueue.setBlockingPop(false);

    if (syncThread->joinable()) {
        syncThread->join();
    }
    delete(syncThread);
    syncThread = nullptr;

    stopSyncFlag = false;
    audioFrameQueue.setBlockingPop(true);
    videoFrameQueue.setBlockingPop(true);
}

bool Player::play() {
    startReadStreamThread();
    startDecodeAudioThread();
    startDecodeVideoThread();
    startSyncThread();
    return true;
}

void Player::pause() {
    stopReadStreamThread();
    stopDecodeAudioThread();
    stopDecodeVideoThread();
    stopSyncThread();
}

int64_t Player::getDurationMS() {
    if (audioStreamIndex >= 0) {
        return audioStreamMap[audioStreamIndex].durationMS;
    } else if (videoStreamIndex >= 0) {
        return videoStreamMap[videoStreamIndex].durationMS;
    } else {
        return -1;
    }
}

int64_t Player::getCurrentPtsMS() {
    return lastAudioPts;
}

bool Player::seek(int64_t ptsMS) {
    seekPtsMS = ptsMS;
    seekReq = true;
}


