//
// Created by 祖国瑞 on 2022/9/7.
//

#include "Player.h"
#include "utils.h"
#include "Log.h"
#include <list>
#include "PacketWrapper.h"
#include "Flags.h"

extern "C" {
#include "FFmpeg/libavcodec/avcodec.h"
#include "FFmpeg/libavutil/hwcontext.h"
#include "FFmpeg/libavutil/display.h"
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

    LOGD(TAG, "release: syncAudioCacheList.size = %d", syncAudioCacheList.size());
    LOGD(TAG, "release: syncVideoCacheList.size = %d", syncVideoCacheList.size());
    while (!syncAudioCacheList.empty()) {
        AudioFrame *af = syncAudioCacheList.front();
        syncAudioCacheList.pop_front();
        delete(af);
        //playerContext.recycleAudioFrame(af);
    }

    while (!syncVideoCacheList.empty()) {
        VideoFrame *vf = syncVideoCacheList.front();
        syncVideoCacheList.pop_front();
        delete(vf);
        //playerContext.recycleVideoFrame(vf);
    }

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

            if (stream->nb_side_data > 0) {
                for (int i = 0; i < stream->nb_side_data; i++) {
                    AVPacketSideData &sd = stream->side_data[i];
                    if (sd.type == AV_PKT_DATA_DISPLAYMATRIX) {
                        int rotate = round(av_display_rotation_get((const int32_t *)sd.data));
                        LOGD(TAG, "videoStream %d rotate = %d", i, rotate);
                        trackInfo.rotate = rotate;
                    }
                }
            }

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
//    enableVideo = false;
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
 * If the packet has some flags like STREAM_FLAG_SEEK, this packet won't
 * contain data.
 * */
void Player::readStreamLoop() {
    LOGI(TAG, "readStreamLoop: start");
    if (!formatCtx) {
        LOGE(TAG, "readStreamLoop: no format context");
        return;
    }
    int ret;
    while (!stopReadFlag) {
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            LOGE(TAG, "readStreamLoop: av_packet_alloc failed");
            return;
        }

        if (seekFlag) {
            LOGD(TAG, "readStreamLoop: AV_TIME_BASE_Q = %lf", av_q2d(AV_TIME_BASE_Q));
            notifyPlayState(PLAY_STATE_SEEK_START);
            if (enableVideo) {
                LOGD(TAG, "readStreamLoop: videoStream timebase = %lf", av_q2d(formatCtx->streams[videoStreamIndex]->time_base));
            }
            if (enableAudio) {
                LOGD(TAG, "readStreamLoop: audioStream timebase = %lf", av_q2d(formatCtx->streams[audioStreamIndex]->time_base));
            }

            int64_t pts = (int64_t) (seekPtsMS / 1000.0f * AV_TIME_BASE);
            int64_t startTime = getSystemClockCurrentMilliseconds();
            //av_seek_frame(formatCtx, -1, pts, AVSEEK_FLAG_BACKWARD);
            avformat_seek_file(formatCtx, -1, INT64_MIN, pts, INT64_MAX, AVSEEK_FLAG_BACKWARD);
            LOGD(TAG, "readStreamLoop: seek cost %ld ms", getSystemClockCurrentMilliseconds() - startTime);


            // put a empty packet width flag STREAM_FLAG_SEEK
            if (enableAudio) {
                audioPacketQueue.clear();
                PacketWrapper *p = playerContext.getEmptyPacketWrapper();
                p->flags = STREAM_FLAG_SEEK;
                audioPacketQueue.forcePushBack(p);
            }

            if (enableVideo) {
                videoPacketQueue.clear();
                PacketWrapper *p = playerContext.getEmptyPacketWrapper();
                p->flags = STREAM_FLAG_SEEK;
                videoPacketQueue.forcePushBack(p);
            }

            seekFlag = false;
        }

        ret = av_read_frame(formatCtx, packet);

        if (packet->side_data_elems > 0) {
            for (int i = 0; i < packet->side_data_elems; i++) {
                AVPacketSideData sd = packet->side_data[i];
                if (sd.type == AV_PKT_DATA_DISPLAYMATRIX) {
                    double rotation = av_display_rotation_get((int32_t *)sd.data);
                    LOGD(TAG, "packet rotation = %lf", rotation);
                }
            }
        }

        if (ret == 0) {
            if (packet->stream_index == audioStreamIndex && enableAudio) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->setParams(packet);
                audioPacketQueue.pushBack(pw);


            } else if (packet->stream_index == videoStreamIndex && enableVideo) {

                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->setParams(packet);
                pw->rotate = videoStreamMap[videoStreamIndex].rotate;
                videoPacketQueue.pushBack(pw);

            } else {
                av_packet_unref(packet);
                av_packet_free(&packet);
            }
        } else if (ret == AVERROR_EOF) {
            LOGD(TAG, "readStreamLoop: meet EOS");
            av_packet_free(&packet);
            packet = nullptr;
            if (enableAudio) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->flags |= STREAM_FLAG_EOS;
                audioPacketQueue.forcePushBack(pw);
            }
            if (enableVideo) {
                PacketWrapper *pw = playerContext.getEmptyPacketWrapper();
                pw->flags |= STREAM_FLAG_EOS;
                videoPacketQueue.forcePushBack(pw);
            }
            break;
        } else if (ret < 0) {
            LOGE(TAG, "readStreamLoop: av_read_packet failed, err = %s", av_err2str(ret));
            av_packet_free(&packet);
            packet = nullptr;
            return;
        }

    }
    LOGI(TAG, "readStreamLoop: end");
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
    LOGI(TAG, "decodeAudioLoop: start");
    int ret;
    optional<PacketWrapper *> packetOpt;
    PacketWrapper *pw = nullptr;
    AVFrame *frame = nullptr;
    AudioFrame *audioFrame = nullptr;
    int64_t lastAudioPts = -1;
    while (!stopDecodeAudioFlag && enableAudio) {

        packetOpt = audioPacketQueue.popFront();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "audio packetOpt has no value");
            break;
        }
        pw = packetOpt.value();
        if ((pw->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
            LOGD(TAG, "decode audio, meet a seek frame");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            audioFrameQueue.clear();
            audioDecoder->flush();
            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->flags |= STREAM_FLAG_SEEK;
            audioFrameQueue.forcePushBack(audioFrame);
            audioFrame = nullptr;
            lastAudioPts = -1;
            continue;
        }

        if ((pw->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS) {
            LOGD(TAG, "decodeAudioLoop: meet EOS");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->flags |= STREAM_FLAG_EOS;
            audioFrameQueue.forcePushBack(audioFrame);
            audioFrame = nullptr;
            break;
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
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }

            audioFrame = playerContext.getEmptyAudioFrame();
            audioFrame->setParams(frame, audioStreamMap[audioStreamIndex].sampleFormat,
                                  formatCtx->streams[audioStreamIndex]->time_base);

            if (lastAudioPts > 0 && lastAudioPts > frame->pts) {
                LOGE(TAG, "decodeAudioLoop: lastAudioPts > frame.pts");
            }
            lastAudioPts = audioFrame->pts;
            //LOGD(TAG, "decodeAudioLoop: audioFrameQueue.size = %d", audioFrameQueue.getSize());
            if (!audioFrameQueue.pushBack(audioFrame)) {
                audioFrameQueue.forcePushBack(audioFrame);
            }
            // DON'T delete AVFrame here, it will be carried to output by AudioFrame
            audioFrame = nullptr;
            frame = nullptr;
        }

        if (pw) {
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
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

    if (audioFrame != nullptr) {
        audioFrameQueue.pushBack(audioFrame, false);
        audioFrame = nullptr;
    }

    LOGI(TAG, "decodeAudioLoop: end");

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
    LOGI(TAG, "decodeVideoLoop: start");
    int ret;
    optional<PacketWrapper *> packetOpt;
    PacketWrapper *pw = nullptr;
    AVFrame *frame = nullptr;
    VideoFrame *videoFrame = nullptr;
    int64_t lastDecodeTime = -1;
    int64_t lastPts = -1;
    while (!stopDecodeVideoFlag && enableVideo) {
        packetOpt = videoPacketQueue.popFront();
        if (!packetOpt.has_value()) {
            LOGE(TAG, "video packetOpt has no value");
            break;
        }
        pw = packetOpt.value();

        if ((pw->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
            LOGD(TAG, "decodeVideoLoop: meet a seek frame");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            videoFrameQueue.clear();
            videoDecoder->flush();
            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->flags |= STREAM_FLAG_SEEK;
            videoFrameQueue.forcePushBack(videoFrame);
            videoFrame = nullptr;
            LOGD(TAG, "decodeVideoLoop: meet a seek frame, seek end");
            continue;
        }

        if ((pw->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS) {
            LOGD(TAG, "decodeVideoLoop: meet EOS");
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->flags |= STREAM_FLAG_EOS;
            videoFrameQueue.forcePushBack(videoFrame);
            videoFrame = nullptr;
            break;
        }

        int64_t startTime1 = getSystemClockCurrentMilliseconds();
        ret = videoDecoder->sendPacket(pw->avPacket);
        if (ret < 0) {
            LOGE(TAG, "decodeVideoLoop: video decoder send packet failed, err = %d", ret);
            break;
        }
        //LOGD(TAG, "decodeVideoLoop: send one packet cost %ld ms", getSystemClockCurrentMilliseconds() - startTime1);

        while (true) {
            //int64_t startTime = getSystemClockCurrentMilliseconds();
            frame = av_frame_alloc();
            ret = videoDecoder->receiveFrame(frame);
            if (ret < 0) {
                av_frame_free(&frame);
                frame = nullptr;
                break;
            }

            videoFrame = playerContext.getEmptyVideoFrame();
            videoFrame->setParams(frame, AVPixelFormat(frame->format),
                                  formatCtx->streams[videoStreamIndex]->time_base);
            videoFrame->rotation = pw->rotate;

#ifdef ENABLE_PERFORMANCE_MONITOR
            int64_t now = getSystemClockCurrentMilliseconds();
            if (lastDecodeTime > 0 && lastPts > 0) {
                if (now - lastDecodeTime > videoFrame->pts - lastPts) {
                    //LOGW(TAG, "decodeVideoLoop, overtime decoding, decodeInterval = %ld, frameInterval = %ld", now - lastDecodeTime, videoFrame->pts - lastPts);
                }
            }
            lastDecodeTime = now;
            lastPts = videoFrame->pts;
            //LOGD(TAG, "decodeVideoLoop: pix_format = %d", videoFrame->pixelFormat);
#endif
            if (!videoFrameQueue.pushBack(videoFrame)) {
                videoFrameQueue.forcePushBack(videoFrame);
            }
            // DON'T delete AVFrame, it will be carried to output by VideoFrame.
            videoFrame = nullptr;
            frame = nullptr;

            //LOGD(TAG, "decodeVideoLoop: receive one frame cost %ld ms", getSystemClockCurrentMilliseconds() - startTime);

        }

        if (pw) {
            playerContext.recyclePacketWrapper(pw);
            pw = nullptr;
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
        videoFrameQueue.pushBack(videoFrame, false);
        videoFrame = nullptr;
    }

    LOGI(TAG, "decodeVideoLoop: end");

}


void Player::syncCallback(void *context) {
    ((Player *) context)->syncLoop();
}

void Player::syncLoop() {
    LOGI(TAG, "syncLoop: start");
    int64_t lastAudioWriteTime = -1;
    int64_t lastVideoWriteTime = -1;

    int64_t lastAudioPts = -1;
    int64_t lastVideoPts = -1;

    notifyPlayState(PLAY_STATE_START);

    AudioFrame *audioFrame = unPlayedAudioFrame;
    unPlayedAudioFrame = nullptr;
    VideoFrame *videoFrame = unPlayedVideoFrame;
    unPlayedVideoFrame = nullptr;

    bool audioSeekFlag = false;
    bool videoSeekFlag = false;

    bool audioEOSFlag = false;
    bool videoEOSFlag = false;

    auto moveAudioFrame = [&]() -> bool {
        optional<AudioFrame *> tempAudioFrame = audioFrameQueue.popFront(false);
        if (tempAudioFrame.has_value()) {
            AudioFrame *af = tempAudioFrame.value();
            // 如果遇到seek标志，就清空cacheList。
            if ((af->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
                audioSeekFlag = true;
                playerContext.recycleAudioFrame(af);
                if (audioFrame != nullptr) {
                    playerContext.recycleAudioFrame(audioFrame);
                    audioFrame = nullptr;
                }
                while (!syncAudioCacheList.empty()) {
                    AudioFrame *f = syncAudioCacheList.front();
                    syncAudioCacheList.pop_front();
                    playerContext.recycleAudioFrame(f);
                }

                if (videoFrame != nullptr) {
                    playerContext.recycleVideoFrame(videoFrame);
                    videoFrame = nullptr;
                }
            } else {
                syncAudioCacheList.push_back(af);
            }
            return true;
        } else {
            return false;
        }
    };

    auto moveVideoFrame = [&]() -> bool {
        optional<VideoFrame *> tempVideoFrame = videoFrameQueue.popFront(false);
        if (tempVideoFrame.has_value()) {
            VideoFrame *vf = tempVideoFrame.value();
            // 遇到了seek，清空cacheList。并且要把当前音频帧也要置空。否则audio一帧很长，等它输出完可能会导致seek卡顿。
            if ((vf->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
                videoSeekFlag = true;
                playerContext.recycleVideoFrame(vf);
                if (videoFrame != nullptr) {
                    playerContext.recycleVideoFrame(videoFrame);
                    videoFrame = nullptr;
                }
                while (!syncVideoCacheList.empty()) {
                    VideoFrame *f = syncVideoCacheList.front();
                    syncVideoCacheList.pop_front();
                    playerContext.recycleVideoFrame(f);
                }

                if (audioFrame != nullptr) {
                    playerContext.recycleAudioFrame(audioFrame);
                    audioFrame = nullptr;
                }
            } else {
                syncVideoCacheList.push_back(vf);
            }
            return true;
        } else {
            return false;
        }
    };

    while (!stopSyncFlag) {

        if (enableAudio && enableVideo) {
            // 分别从videoFrameQueue和audioFrameQueue中获取一帧，并放入cacheList中。避免由于一个队列阻塞，而
            // 另一个队列迟迟无法获取到AVPacket。这样会导致死锁。

            // 通常来说，一个audioFrame会对应多个videoFrame。因此不能简单每次都move一个audioFrame，否则cacheList
            // 会变得很大

            // 如果视频帧为null，说明需要获取新的了。
            // 如果视频流已经遇到了seek标志，那么就不再移动，等待其他流遇到seek。
            if (videoFrame == nullptr && !videoSeekFlag) {
                // 移动一帧视频到cacheList里，如果失败，说明后面的视频还未解码好，或者音频队列满，导致视频无法
                // 继续解码
                if (!moveVideoFrame()) {
                    // 如果audio队列已经满了，那可能是阻塞了。那就移动一帧音频。否则可能只是单纯还没解码好，就继续循环
                    if (audioFrameQueue.isFull()) {
                        moveAudioFrame();
                    }
                }
            }

            // 音频和视频逻辑一样
            if (audioFrame == nullptr && !audioSeekFlag) {
                if (!moveAudioFrame()) {
                    if (videoFrameQueue.isFull()) {
                        moveVideoFrame();
                    }
                }
            }

            // 如果两个流都遇到了seek，那么seek完成，可以进行下面的播放
            if (audioSeekFlag && videoSeekFlag) {
                audioSeekFlag = false;
                videoSeekFlag = false;
                notifyPlayState(PLAY_STATE_SEEK_COMPLETE);
            } else if (audioSeekFlag || videoSeekFlag)  {
                // 如果只有一个流获取到了seek标志，那么就要继续等待另外的流也获取到seek。
                continue;
            }


            if (audioFrame == nullptr && !audioEOSFlag) {
                if (!syncAudioCacheList.empty()) {
                    audioFrame = syncAudioCacheList.front();
                    syncAudioCacheList.pop_front();
                    audioEOSFlag = (audioFrame->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS;
                    if (audioEOSFlag) {
                        playerContext.recycleAudioFrame(audioFrame);
                        audioFrame = nullptr;
                        LOGD(TAG, "syncLoop: audio meet EOS");
                    }
                    //LOGD(TAG, "get a audioFrame, audioFrameQueue.size = %d", audioFrameQueue.getSize());
                } else {
                    //LOGD(TAG, "audioFrame is null, waiting... videoFrameQueue.size = %d", videoFrameQueue.getSize());
                    continue;
                }
            }

            if (videoFrame == nullptr && !videoEOSFlag) {
                if (!syncVideoCacheList.empty()) {
                    videoFrame = syncVideoCacheList.front();
                    syncVideoCacheList.pop_front();
                    videoEOSFlag = (videoFrame->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS;
                    if (videoEOSFlag) {
                        playerContext.recycleVideoFrame(videoFrame);
                        videoFrame = nullptr;
                        LOGD(TAG, "syncLoop: video meet EOS");
                    }
                    //LOGD(TAG, "get a video, videoFrameQueue.size = %d", videoFrameQueue.getSize());
                } else {
                    //LOGD(TAG, "videoFrame is null, waiting... audioFrameQueue.size = %d", audioFrameQueue.getSize());
                    continue;
                }
            }

            if (!audioEOSFlag && !videoEOSFlag) {
                // 音频和视频流都正常
                if (lastAudioPts == -1) {
                    lastAudioPts = audioFrame->pts;
                }

                if (lastVideoPts == -1) {
                    lastVideoPts = videoFrame->pts;
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

                notifyPlayProgress(lastAudioPts);

            } else if (!audioEOSFlag) {
                // 视频流已经EOS，只处理音频流。
                audioFrame->outputFrameCount = audioFrame->numFrames - audioFrame->outputStartIndex;
                lastAudioPts = audioFrame->getOutputPts();
                audioOutput->write(audioFrame);
                audioFrame->outputStartIndex += audioFrame->outputFrameCount;
                if (audioFrame->outputStartIndex >= audioFrame->numFrames) {
                    playerContext.recycleAudioFrame(audioFrame);
                    audioFrame = nullptr;
                }
                notifyPlayProgress(lastAudioPts);
            } else if (!videoEOSFlag) {
                // 音频流已经EOS，只处理视频流
                int64_t now = getSystemClockCurrentMilliseconds();
                if (lastVideoWriteTime > 0 && lastVideoPts > 0) {
                    // LOGD(TAG, "syncLoop: frameInterval = %ld ms, writeInterval = %ld ms", videoFrame->pts - lastVideoPts, now - lastVideoWriteTime);
                    if (now - lastVideoWriteTime > videoFrame->pts - lastVideoPts) {
#ifdef EMANLE_PERFORMACE_MONITOR
                        LOGW(TAG, "syncLoop, video frame delay, frameInterval = %ld ms, actualInterval = %ld ms", videoFrame->pts - lastVideoPts, now - lastVideoWriteTime);
#endif
                    } else {
                        //int64_t sleepStart = getSystemClockCurrentMilliseconds();
                        this_thread::sleep_for(chrono::microseconds(((videoFrame->pts - lastVideoPts) - (now - lastVideoWriteTime)) * 1000));
                        //LOGD(TAG, "syncLoop: sleep %ld ms", getSystemClockCurrentMilliseconds() - sleepStart);
                    }
                }
                videoOutput->write(videoFrame);
                lastVideoPts = videoFrame->pts;
                lastVideoWriteTime = getSystemClockCurrentMilliseconds();
                videoFrame = nullptr;
            } else {
                LOGD(TAG, "syncLoop: both audio and video meet EOS, break");
                break;
            }
        } else if (enableVideo) {
            if (videoFrame == nullptr) {
                optional<VideoFrame *> frameOpt = videoFrameQueue.popFront();
                if (frameOpt.has_value()) {
                    videoFrame = frameOpt.value();
                } else {
                    break;
                }
            }
            if ((videoFrame->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
                LOGD(TAG, "syncLoop, meet video seek frame");
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                lastVideoWriteTime = -1;
                lastVideoPts = -1;
                continue;
            }

            if ((videoFrame->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS) {
                LOGD(TAG, "syncLoop: meet video EOS");
                videoEOSFlag = true;
                playerContext.recycleVideoFrame(videoFrame);
                videoFrame = nullptr;
                break;
            }

            int64_t now = getSystemClockCurrentMilliseconds();
            if (lastVideoWriteTime > 0 && lastVideoPts > 0) {
                // LOGD(TAG, "syncLoop: frameInterval = %ld ms, writeInterval = %ld ms", videoFrame->pts - lastVideoPts, now - lastVideoWriteTime);
                if (now - lastVideoWriteTime > videoFrame->pts - lastVideoPts) {
#ifdef EMANLE_PERFORMACE_MONITOR
                    LOGW(TAG, "syncLoop, video frame delay, frameInterval = %ld ms, actualInterval = %ld ms", videoFrame->pts - lastVideoPts, now - lastVideoWriteTime);
#endif
                } else {
                    //int64_t sleepStart = getSystemClockCurrentMilliseconds();
                    this_thread::sleep_for(chrono::microseconds(((videoFrame->pts - lastVideoPts) - (now - lastVideoWriteTime)) * 1000));
                    //LOGD(TAG, "syncLoop: sleep %ld ms", getSystemClockCurrentMilliseconds() - sleepStart);
                }
            }
            videoOutput->write(videoFrame);
            lastVideoPts = videoFrame->pts;
            lastVideoWriteTime = getSystemClockCurrentMilliseconds();
            videoFrame = nullptr;

            notifyPlayProgress(lastVideoPts);
        } else if (enableAudio) {
            if (audioFrame == nullptr) {
                //LOGD(TAG, "syncLoop: audioFrameQueue.size = %d", audioFrameQueue.getSize());
                optional<AudioFrame *> frameOpt = audioFrameQueue.popFront();
                if (frameOpt.has_value()) {
                    audioFrame = frameOpt.value();
                } else {
                    break;
                }
            }
            if ((audioFrame->flags & STREAM_FLAG_SEEK) == STREAM_FLAG_SEEK) {
                LOGD(TAG, "syncLoop, meet audio seek frame");
                playerContext.recycleAudioFrame(audioFrame);
                audioFrame = nullptr;
                lastAudioPts = -1;
                lastAudioWriteTime = -1;
                continue;
            }
            if ((audioFrame->flags & STREAM_FLAG_EOS) == STREAM_FLAG_EOS) {
                LOGD(TAG, "syncLoop: audio meet EOS");
                audioEOSFlag = true;
                playerContext.recycleAudioFrame(audioFrame);
                audioFrame = nullptr;
                break;
            }
//            if (lastAudioPts > 0 && lastAudioPts > audioFrame->pts) {
//                LOGE(TAG, "syncLoop: lastAudioPts > audioFrame.pts");
//            }
            lastAudioPts = audioFrame->pts;
            audioFrame->outputFrameCount = audioFrame->numFrames;
            audioOutput->write(audioFrame);
            playerContext.recycleAudioFrame(audioFrame);
            audioFrame = nullptr;
            notifyPlayProgress(lastAudioPts);
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
    if (enableAudio && enableVideo) {
        if (audioEOSFlag && videoEOSFlag) {
            playerContext.post(bind(&Player::pause, this));
            notifyPlayState(PLAY_STATE_COMPLETE);
        } else {
            notifyPlayState(PLAY_STATE_PAUSE);
        }
    } else {
        if (audioEOSFlag || videoEOSFlag) {
            playerContext.post(bind(&Player::pause, this));
            notifyPlayState(PLAY_STATE_COMPLETE);
        } else {
            notifyPlayState(PLAY_STATE_PAUSE);
        }
    }

    LOGI(TAG, "syncLoop: end");
}

void Player::startReadStreamThread() {
    unique_lock<mutex> mLock(readStreamMu);
    if (readStreamThread != nullptr) {
        return;
    }
    audioPacketQueue.setBlockPush(true);
    videoPacketQueue.setBlockPush(true);
    stopReadFlag = false;
    readStreamThread = new thread(readStreamCallback, this);

}


void Player::stopReadStreamThread() {
    unique_lock<mutex> mLock(readStreamMu);
    if (readStreamThread == nullptr) {
        return;
    }
    stopReadFlag = true;
    audioPacketQueue.setBlockPush(false);
    videoPacketQueue.setBlockPush(false);

    if (readStreamThread->joinable()) {
        readStreamThread->join();
    }
    delete (readStreamThread);
    readStreamThread = nullptr;

    audioPacketQueue.setBlockPush(true);
    videoPacketQueue.setBlockPush(true);
    stopReadFlag = false;
}

void Player::startDecodeAudioThread() {
    unique_lock<mutex> mLock(decodeAudioMu);
    if (decodeAudioThread != nullptr) {
        return;
    }

    stopDecodeAudioFlag = false;
    audioPacketQueue.setBlockPop(true);
    audioFrameQueue.setBlockPush(true);

    decodeAudioThread = new thread(decodeAudioCallback, this);
}

void Player::stopDecodeAudioThread() {
    unique_lock<mutex> mLock(decodeAudioMu);
    if (decodeAudioThread == nullptr) {
        return;
    }
    stopDecodeAudioFlag = true;
    audioPacketQueue.setBlockPop(false);
    audioFrameQueue.setBlockPush(false);

    if (decodeAudioThread->joinable()) {
        decodeAudioThread->join();
    }

    delete (decodeAudioThread);
    decodeAudioThread = nullptr;

    stopDecodeAudioFlag = false;
    audioPacketQueue.setBlockPop(true);
    audioFrameQueue.setBlockPush(true);
}

void Player::startDecodeVideoThread() {
    unique_lock<mutex> mLock(decodeVideoMu);
    if (decodeVideoThread != nullptr) {
        return;
    }

    stopDecodeVideoFlag = false;
    videoPacketQueue.setBlockPop(true);
    videoFrameQueue.setBlockPush(true);

    decodeVideoThread = new thread(decodeVideoCallback, this);

}

void Player::stopDecodeVideoThread() {
    unique_lock<mutex> mLock(decodeVideoMu);
    if (decodeVideoThread == nullptr) {
        return;
    }

    stopDecodeVideoFlag = true;
    videoPacketQueue.setBlockPop(false);
    videoFrameQueue.setBlockPush(false);

    if (decodeVideoThread->joinable()) {
        decodeVideoThread->join();
    }

    delete (decodeVideoThread);
    decodeVideoThread = nullptr;

    stopDecodeVideoFlag = false;
    videoPacketQueue.setBlockPop(true);
    videoFrameQueue.setBlockPush(true);
    LOGD(TAG, "stopDecodeVideoThread, thread = 0x%x", readStreamThread);
}

void Player::startSyncThread() {
    unique_lock<mutex> mLock(syncMu);
    if (syncThread != nullptr) {
        return;
    }

    stopSyncFlag = false;
    audioFrameQueue.setBlockPop(true);
    videoFrameQueue.setBlockPop(true);

    syncThread = new thread(syncCallback, this);
    LOGD(TAG, "startSyncThread, thread = 0x%x", readStreamThread);
}

void Player::stopSyncThread() {
    unique_lock<mutex> mLock(syncMu);
    if (syncThread == nullptr) {
        return;
    }
    stopSyncFlag = true;
    audioFrameQueue.setBlockPop(false);
    videoFrameQueue.setBlockPop(false);

    if (syncThread->joinable()) {
        syncThread->join();
    }
    delete (syncThread);
    syncThread = nullptr;

    stopSyncFlag = false;
    audioFrameQueue.setBlockPop(true);
    videoFrameQueue.setBlockPop(true);
}

bool Player::play() {
    LOGD(TAG, "play");
    startReadStreamThread();
    startDecodeAudioThread();
    startDecodeVideoThread();
    startSyncThread();
    return true;
}

void Player::pause() {
    LOGD(TAG, "pause");
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


void Player::notifyPlayState(int state) {
    if (stateListener != nullptr) {
        stateListener->playStateChanged(state);
    }
}

void Player::notifyPlayProgress(int64_t ptsMS) {
    if (stateListener != nullptr) {
        stateListener->progressChanged(ptsMS);
    }
}
