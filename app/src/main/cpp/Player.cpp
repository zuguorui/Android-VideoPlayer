//
// Created by 祖国瑞 on 2022/9/7.
//

#include "Player.h"
#include "Util.h"
#include "Log.h"
#include <list>
#include "PacketWrapper.h"
#include "Flags.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
#include "FFmpeg/libavutil/hwcontext.h"

}

#define TAG "Player"

using namespace std;

void printFFmpegSupportedCodec() {
    // 打印FFmpeg支持的codec
    void *opaque = nullptr;
    while (true) {
        const AVCodec *c_temp = av_codec_iterate(&opaque);
        if (c_temp == nullptr) {
            break;
        }
        char info[100] = {0};
        if (av_codec_is_decoder(c_temp)) {
            sprintf(info, "%s[Dec]", info);
        } else if (av_codec_is_encoder(c_temp)) {
            sprintf(info, "%s[Enc]", info);
        } else {
            sprintf(info, "%s[XXX]", info);
        }

        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s %10s", info, c_temp->name);
        LOGD(TAG, "%s", info);
    }
}


Player::Player() {
    av_log_set_callback(ffmpegLogCallback);
    //printFFmpegSupportedCodec();
}

Player::~Player() {
    LOGD(TAG, "~Player");
    release();
}


void Player::setWindow(void *window) {
    nativeWindow = window;
}

void Player::release() {
    LOGD(TAG, "release");
    pause();
    if (videoDecoder) {
        LOGD(TAG, "delete videoDecoder");
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

    releaseAudioOutput();
    releaseVideoOutput();
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

    durationMS = (int64_t) (formatCtx->duration * 1.0f / AV_TIME_BASE * 1000);
    int64_t totalSeconds = durationMS / 1000;
    int64_t seconds = totalSeconds % 60;
    int64_t totalMinutes = totalSeconds / 60;
    int64_t minutes = totalMinutes % 60;
    int64_t hours = totalMinutes / 60;

    LOGD(TAG, "duration = %02ld:%02ld:%02ld", hours, minutes, seconds);

    for (int i = 0; i < formatCtx->nb_streams; i++) {
        AVStream *stream = formatCtx->streams[i];
        AVMediaType type = stream->codecpar->codec_type;
        if (type == AVMEDIA_TYPE_AUDIO) {
            StreamInfo trackInfo;
            trackInfo.streamIndex = i;
            trackInfo.type = type;

            trackInfo.channels = stream->codecpar->channels;
            trackInfo.sampleRate = stream->codecpar->sample_rate;
            trackInfo.sampleFormat = static_cast<AVSampleFormat>(stream->codecpar->format);
            audioStreamMap[i] = trackInfo;
        } else if (type == AVMEDIA_TYPE_VIDEO) {

            StreamInfo trackInfo;
            trackInfo.streamIndex = i;
            trackInfo.type = type;
            trackInfo.width = stream->codecpar->width;
            trackInfo.height = stream->codecpar->height;
            trackInfo.fps = (float) av_q2d(stream->avg_frame_rate);
            trackInfo.pixelFormat = static_cast<AVPixelFormat>(stream->codecpar->format);
            videoStreamMap[i] = trackInfo;
            LOGD(TAG, "videoStream %d: pixelFormat = %d, width = %d, height = %d, fps = %f", i, stream->codecpar->format, trackInfo.width, trackInfo.height, trackInfo.fps);
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
        enableVideo = false;
        enableAudio = false;
        return false;
    }

    enableAudio = audioStreamIndex >= 0;

    if (audioStreamIndex != -1) {
        LOGD(TAG, "select audio index = %d, detail:", audioStreamIndex);
        av_dump_format(formatCtx, audioStreamIndex, nullptr, 0);
        createAudioOutput();
    } else {
        LOGE(TAG, "no available audio stream found");
    }

    enableVideo = videoStreamIndex >= 0;

    if (videoStreamIndex != -1) {
        LOGD(TAG, "select video index = %d, detail:", videoStreamIndex);
        av_dump_format(formatCtx, videoStreamIndex, nullptr, 0);
        createVideoOutput();
    } else {
        LOGE(TAG, "no available video stream found");
    }

    LOGD(TAG, "AV_TIME_BASE_Q = %lf", av_q2d(AV_TIME_BASE_Q));
    if (enableVideo) {
        LOGD(TAG, "videoStream timebase = %lf", av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
    }
    if (enableAudio) {
        LOGD(TAG, "audioStream timebase = %lf", av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
    }

    enableVideo = false;
//    enableAudio = false;
    return true;
}

/*
 * find a suitable stream and get a decoder for it
 * */
void Player::findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, IDecoder **decoder,
                                           int *streamIndex) {
    if (!formatCtx) {
        return;
    }
    for (auto it = streams.begin(); it != streams.end(); it++) {
        AVStream *stream = formatCtx->streams[it->first];
        *decoder = findDecoder(stream->codecpar);
        if (!(*decoder)) {
            LOGE(TAG, "finding decoder for stream %d failed", it->first);
            it->second.codecType = CodecType::NOT_SUPPORTED;
        } else {
            it->second.codecType = (*decoder)->getCodecType();
            LOGD(TAG, "find a decoder for stream %d, decoder: {name = %s, type = %d}",
                 it->first, (*decoder)->getName(), (*decoder)->getCodecType());
            if (it->second.type == AVMediaType::AVMEDIA_TYPE_VIDEO) {
                LOGD(TAG, "find a video decoder, pixel_format = %d", (*decoder)->getPixelFormat());
            }
        }

        if (*decoder) {
            *streamIndex = it->first;
            break;
        }
    }
}


void Player::readStreamCallback(void *context) {
    ((Player *) context)->readStreamLoop();
}

/*
 * Read packet data from source.
 * If the packet has some flags like STREAM_FLAG_SOUGHT, this packet won't
 * contain data.
 * */
void Player::readStreamLoop() {
    LOGD(TAG, "start readStreamLoop");
    if (!formatCtx) {
        LOGE(TAG, "no format context");
        return;
    }
    int ret;
    bool pushSuccess = false;
    while (!stopReadFlag) {
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            LOGE(TAG, "av_packet_alloc failed");
            return;
        }

        if (seekFlag) {
            LOGD(TAG, "AV_TIME_BASE_Q = %lf", av_q2d(AV_TIME_BASE_Q));
            if (enableVideo) {
                LOGD(TAG, "videoStream timebase = %lf", av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
            }
            if (enableAudio) {
                LOGD(TAG, "audioStream timebase = %lf", av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
            }
//            int streamIndex = -1;
//            if (audioStreamIndex >= 0) {
//                pts = (int64_t)(seekPtsMS / av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
//                streamIndex = audioStreamIndex;
//            } else if (videoStreamIndex >= 0) {
//                pts = (int64_t)(seekPtsMS / av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
//                streamIndex = videoStreamIndex;
//            }
            if (enableVideo && enableAudio) {
                int64_t pts = (int64_t) (seekPtsMS / 1000.0f * AV_TIME_BASE);
                av_seek_frame(formatCtx, -1, pts, AVSEEK_FLAG_BACKWARD);
            } else if (enableVideo) {
                int64_t pts = (int64_t)(seekPtsMS / 1000 / av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
                av_seek_frame(formatCtx, videoStreamIndex, pts, AVSEEK_FLAG_BACKWARD);
            } else {
                int64_t pts = (int64_t)(seekPtsMS / 1000 / av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
                av_seek_frame(formatCtx, audioStreamIndex, pts, AVSEEK_FLAG_BACKWARD);
            }


            // put a empty packet width flag STREAM_FLAG_SOUGHT
            if (enableAudio) {
                audioPacketQueue.clear();
                PacketWrapper *p = playerContext.getEmptyPacketWrapper();
                p->flags = STREAM_FLAG_SOUGHT;
                audioPacketQueue.forcePush(p);
//                audioDecodeSeekFlag = true;
            }

            if (enableVideo) {
                videoPacketQueue.clear();
                PacketWrapper *p = playerContext.getEmptyPacketWrapper();
                p->flags = STREAM_FLAG_SOUGHT;
                videoPacketQueue.forcePush(p);
//                videoDecodeSeekFlag = true;
            }

//            syncSeekFlag = true;

            seekFlag = false;
        }

        ret = av_read_frame(formatCtx, packet);

        if (ret == 0) {
            if (packet->stream_index == audioStreamIndex && enableAudio) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->setParams(packet);
                if (videoPacketQueue.getSize() == 0) {
                    audioPacketQueue.forcePush(pw);
                } else {
                    pushSuccess = audioPacketQueue.push(pw);
                    if (!pushSuccess) {
                        audioPacketQueue.forcePush(pw);
                    }
                }

            } else if (packet->stream_index == videoStreamIndex && enableVideo) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->setParams(packet);
                if (audioPacketQueue.getSize() == 0) {
                    videoPacketQueue.forcePush(pw);
                } else {
                    pushSuccess = videoPacketQueue.push(pw);
                    if (!pushSuccess) {
                        videoPacketQueue.forcePush(pw);
                    }
                }
            } else {
                av_packet_unref(packet);
                av_packet_free(&packet);
            }
        } else if (ret == AVERROR_EOF) {
            av_packet_free(&packet);
            packet = nullptr;
            if (enableAudio) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                audioPacketQueue.forcePush(pw);
            }
            if (enableVideo) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                videoPacketQueue.forcePush(pw);
            }
        } else if (ret < 0) {
            LOGE(TAG, "av_read_frame failed");
            av_packet_free(&packet);
            packet = nullptr;
            return;
        }


    }
}

void Player::decodeAudioCallback(void *context) {
    ((Player *) context)->decodeAudioLoop();
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
    optional<PacketWrapper *> packetOpt;
    PacketWrapper *pw = nullptr;
    AVFrame *frame = nullptr;
    AudioFrame *audioFrame = nullptr;
    while (!stopDecodeAudioFlag && enableAudio) {

        packetOpt = audioPacketQueue.pop();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "audio packetOpt has no value");
            break;
        }
        pw = packetOpt.value();

        if ((pw->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
            LOGD(TAG, "decode audio, meet a seek frame");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            audioFrameQueue.clear();
            audioDecoder->flush();
            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->flags |= STREAM_FLAG_SOUGHT;
            audioFrameQueue.forcePush(audioFrame);
            audioFrame = nullptr;
            continue;
        }

        ret = audioDecoder->sendPacket(pw->avPacket);
        if (ret < 0) {
            LOGE(TAG, "audio decoder send packet failed, err = %d, msg = %s", ret, av_err2str(ret));
            break;
        }

        while (true) {
            frame = av_frame_alloc();
            ret = audioDecoder->receiveFrame(frame);
            if (ret < 0) {
//                LOGE(TAG, "decodeAudioLoop: receiveFrame failed, ret = %d, error = %s", ret,
//                     av_err2str(ret));
                av_frame_unref(frame);
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }
            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->setParams(frame, audioStreamMap[audioStreamIndex].sampleFormat,
                                  formatCtx->streams[audioStreamIndex]->time_base);
            if (!audioFrameQueue.push(audioFrame)) {
                audioFrameQueue.push(audioFrame, false);
            }
            // DON'T delete AVFrame here, it will be carried to output by AudioFrame
            audioFrame = nullptr;
            frame = nullptr;
        }

        if (ret == AVERROR(EAGAIN)) {
//            LOGD(TAG, "audio stream again");
            continue;
        } else if (ret == AVERROR_EOF) {
//            LOGD(TAG, "audio stream meets eof");
            break;
        } else {
//            LOGE(TAG, "audio decoder error: %d", ret);
            break;
        }
    }

    if (pw) {
        playerContext.recyclePacketWrapper(pw);
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
    ((Player *) context)->decodeVideoLoop();
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
    optional<PacketWrapper *> packetOpt;
    PacketWrapper *pw = nullptr;
    AVFrame *frame = nullptr;
    VideoFrame *videoFrame = nullptr;
    int64_t lastDecodeTime = -1;
    int64_t lastPts = -1;
    while (!stopDecodeVideoFlag && enableVideo) {

        packetOpt = videoPacketQueue.pop();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "video packetOpt has no value");
            break;
        }
        pw = packetOpt.value();



        if ((pw->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
            LOGD(TAG, "decode video, meet a seek frame");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            videoFrameQueue.clear();
            videoDecoder->flush();
            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->flags |= STREAM_FLAG_SOUGHT;
            videoFrameQueue.forcePush(videoFrame);
            videoFrame = nullptr;
            continue;
        }

        int64_t startTime1 = getSystemClockCurrentMilliseconds();
        ret = videoDecoder->sendPacket(pw->avPacket);
        if (ret < 0) {
            LOGE(TAG, "decodeVideoLoop: video decoder send packet failed, err = %d", ret);
            break;
        }
        //LOGD(TAG, "decodeVideoLoop: send one packet cost %ld ms", getSystemClockCurrentMilliseconds() - startTime1);

        while (true) {
            int64_t startTime = getSystemClockCurrentMilliseconds();
            frame = av_frame_alloc();
            ret = videoDecoder->receiveFrame(frame);
            if (ret < 0) {
                av_frame_unref(frame);
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }

            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->setParams(frame, AVPixelFormat(frame->format),
                                  formatCtx->streams[videoStreamIndex]->time_base);
#ifdef ENABLE_PERFORMANCE_MONITOR
            int64_t now = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
            if (lastDecodeTime > 0 && lastPts > 0) {
                if (now - lastDecodeTime > videoFrame->pts - lastPts) {
                    //LOGW(TAG, "decodeVideoLoop, overtime decoding, decodeInterval = %ld, frameInterval = %ld", now - lastDecodeTime, videoFrame->pts - lastPts);
                }
            }
            lastDecodeTime = now;
            lastPts = videoFrame->pts;
            //LOGD(TAG, "decodeVideoLoop: pix_format = %d", videoFrame->pixelFormat);
#endif
            if (!videoFrameQueue.push(videoFrame)) {
                videoFrameQueue.push(videoFrame, false);
            }
            // DON'T delete AVFrame, it will be carried to output by VideoFrame.
            videoFrame = nullptr;
            frame = nullptr;

            //LOGD(TAG, "decodeVideoLoop: receive one frame cost %ld ms", getSystemClockCurrentMilliseconds() - startTime);

        }

        if (ret == AVERROR(EAGAIN)) {
//            LOGD(TAG, "video stream again");
            continue;
        } else if (ret == AVERROR_EOF) {
//            LOGD(TAG, "video stream meets eof");
            break;
        } else {
//            LOGE(TAG, "video decoder error: %d", ret);
            break;
        }
    }

    if (pw) {
        playerContext.recyclePacketWrapper(pw);
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
    ((Player *) context)->syncLoop();
}

void Player::syncLoop() {
    int64_t lastAudioWriteTime = -1;
    int64_t lastVideoWriteTime = -1;

    int64_t lastAudioPts = -1;
    int64_t lastVideoPts = -1;

    if (stateListener != nullptr) {
        stateListener->playStateChanged(true);
    }

    AudioFrame *audioFrame = unPlayedAudioFrame;
    unPlayedAudioFrame = nullptr;
    VideoFrame *videoFrame = unPlayedVideoFrame;
    unPlayedVideoFrame = nullptr;

    while (!stopSyncFlag) {
        if (enableAudio && enableVideo) {
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

            if (lastAudioPts == -1) {
                lastAudioPts = audioFrame->pts;
            }

            if (lastVideoPts == -1) {
                lastVideoPts = videoFrame->pts;
            }

            if ((audioFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT
                    && (videoFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
                LOGD(TAG, "syncLoop, meet both audio and video seek frame");
                playerContext.recycleAudioFrame(audioFrame);
                audioFrame = nullptr;
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                continue;
            } else if ((audioFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
                LOGD(TAG, "syncLoop, meet audio seek frame");
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                continue;
            } else if ((videoFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
                LOGD(TAG, "syncLoop, meet video seek frame");
                playerContext.recycleAudioFrame(audioFrame);
                audioFrame = nullptr;
                continue;
            }
            int64_t audioOutputPts = audioFrame->getOutputPts();
            if (videoFrame->pts <= audioOutputPts) {
                lastVideoPts = videoFrame->pts;
                videoOutput->write(videoFrame);
                videoFrame = nullptr;
            } else {
                int64_t outputFrames = (videoFrame->pts + 3 - audioOutputPts) * 1.0f / 1000 * audioFrame->sampleRate;
                outputFrames = min((int64_t)(audioFrame->numFrames - audioFrame->outputStartIndex), outputFrames);
                audioFrame->outputFrameCount = outputFrames;
                lastAudioPts = audioOutputPts;
                audioOutput->write(audioFrame);
                audioFrame->outputStartIndex += audioFrame->outputFrameCount;
                if (audioFrame->outputStartIndex == audioFrame->numFrames) {
                    playerContext.recycleAudioFrame(audioFrame);
                    audioFrame = nullptr;
                }

            }

            if (stateListener != nullptr) {
                stateListener->progressChanged(lastAudioPts, false);
            }

        } else if (enableVideo) {
            if (videoFrame == nullptr) {
                optional<VideoFrame *> frameOpt = videoFrameQueue.pop();
                if (frameOpt.has_value()) {
                    videoFrame = frameOpt.value();
                } else {
                    break;
                }
            }
            if ((videoFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
                LOGD(TAG, "syncLoop, meet video seek frame");
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                continue;
            }

            //int32_t fps = videoStreamMap[videoStreamIndex].fps;
            //LOGD(TAG, "fps = %d", fps);
            //int64_t frameInterval = 1000 / fps;

            int64_t now = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now()).time_since_epoch().count();
            if (lastVideoWriteTime > 0 && lastVideoPts > 0) {
                if (now - lastVideoWriteTime > videoFrame->pts - lastVideoPts) {
#ifdef EMANLE_PERFORMACE_MONITOR
                    LOGW(TAG, "syncLoop, video frame delay, frameInterval = %ld ms, actualInterval = %ld ms", videoFrame->pts - lastVideoPts, now - lastVideoWriteTime);
#endif
                } else {
                    this_thread::sleep_for(chrono::milliseconds((videoFrame->pts - lastVideoPts) - (now - lastVideoWriteTime)));
                }
            }
            lastVideoPts = videoFrame->pts;
            lastVideoWriteTime = now;
            videoOutput->write(videoFrame);
            videoFrame = nullptr;

            if (stateListener != nullptr) {
                stateListener->progressChanged(lastVideoPts, false);
            }
        } else if (enableAudio) {
            if (audioFrame == nullptr) {
                optional<AudioFrame *> frameOpt = audioFrameQueue.pop();
                if (frameOpt.has_value()) {
                    audioFrame = frameOpt.value();
                } else {
                    break;
                }
            }
            if ((audioFrame->flags & STREAM_FLAG_SOUGHT) == STREAM_FLAG_SOUGHT) {
                LOGD(TAG, "syncLoop, meet audio seek frame");
                playerContext.recycleAudioFrame(audioFrame);
                audioFrame = nullptr;
                continue;
            }
            lastAudioPts = audioFrame->pts;
            audioFrame->outputFrameCount = audioFrame->numFrames;
            audioOutput->write(audioFrame);
            playerContext.recycleAudioFrame(audioFrame);
            audioFrame = nullptr;
            if (stateListener != nullptr) {
                stateListener->progressChanged(lastAudioPts, false);
            }
        } else {
            LOGE(TAG, "syncLoop, both audio and video disabled, break");
            break;
        }

    }

    if (audioFrame) {
        unPlayedAudioFrame = audioFrame;
    }

    if (videoFrame) {
        unPlayedVideoFrame = videoFrame;
    }

    if (stateListener != nullptr) {
        stateListener->playStateChanged(false);
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
    delete (readStreamThread);
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

    delete (decodeAudioThread);
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

    delete (decodeVideoThread);
    decodeVideoThread = nullptr;

    stopDecodeVideoFlag = false;
    videoPacketQueue.setBlockingPop(true);
    videoFrameQueue.setBlockingPush(true);
    LOGD(TAG, "stopDecodeVideoThread, thread = 0x%x", readStreamThread);
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
    LOGD(TAG, "startSyncThread, thread = 0x%x", readStreamThread);
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
    delete (syncThread);
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
    return durationMS;
}

int64_t Player::getCurrentPtsMS() {
    return lastAudioPts;
}

bool Player::seek(int64_t ptsMS) {
    seekPtsMS = ptsMS;
    seekFlag.store(true);
    return true;
}

bool Player::setScreenSize(int width, int height) {
    screenWidth = width;
    screenHeight = height;
    if (videoOutput != nullptr) {
        videoOutput->setScreenSize(width, height);
    }
    return true;
}

void Player::setPlayStateListener(IPlayStateListener *listener) {
    stateListener = listener;
}

void Player::removePlayStateListener() {
    stateListener = nullptr;
}


bool Player::isPlaying() {
    return syncThread != nullptr && stopSyncFlag == false;
}

bool Player::createAudioOutput() {
    if (audioDecoder == nullptr) {
        LOGE(TAG, "no audio decoder, can't create audio output");
        return false;
    }
    if (audioOutput != nullptr) {
        return false;
    }
    audioOutput = getAudioOutput(&playerContext);
    if (!audioOutput) {
        LOGE(TAG, "getAudioOutput returns null");
        release();
        return false;
    }
    AVCodecParameters *params = formatCtx->streams[audioStreamIndex]->codecpar;
    if (!audioOutput->create(params->sample_rate, params->channels,
                             static_cast<AVSampleFormat>(params->format))) {
        LOGE(TAG, "audio output setFormat failed, sampleRate = %d, numChannels = %d, sampleFormat = %d",
             params->sample_rate, params->channels, params->format);
        release();
        return false;
    }
    audioOutput->start();
    return true;
}

void Player::releaseAudioOutput() {
    if (audioOutput == nullptr) {
        return;
    }
    audioOutput->release();
    delete(audioOutput);
    audioOutput = nullptr;
}

bool Player::createVideoOutput() {
    if (videoDecoder == nullptr) {
        LOGE(TAG, "no video decoder, can't create video output");
        return false;
    }
    if (nativeWindow == nullptr) {
        LOGE(TAG, "no surface set");
        return false;
    }
    if (videoOutput != nullptr) {
        return false;
    }
    videoOutput = getVideoOutput(&playerContext);
    if (!videoOutput) {
        LOGE(TAG, "getVideoOutput returns null");
        release();
        return false;
    }
    if (!videoOutput->create(nativeWindow)) {
        LOGE(TAG, "videoOutput create failed");
        release();
        return false;
    }

    videoOutput->setFormat(videoDecoder->getPixelFormat(), AVColorSpace::AVCOL_SPC_BT709, false);
    videoOutput->setSizeMode(SizeMode::FIT);
    videoOutput->setScreenSize(screenWidth, screenHeight);

    return true;
}

void Player::releaseVideoOutput() {
    if (videoOutput == nullptr) {
        return;
    }
    videoOutput->release();
    delete(videoOutput);
    videoOutput = nullptr;
}

IDecoder *Player::findDecoder(AVCodecParameters *params) {
    FFmpegDecoder *decoder = new FFmpegDecoder();
    if (!decoder->init(params, PreferCodecType::NONE)) {
        delete decoder;
        decoder = nullptr;
    }
    return decoder;
}

