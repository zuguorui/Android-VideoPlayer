//
// Created by 祖国瑞 on 2022/9/7.
//

#ifndef ANDROID_VIDEOPLAYER_PLAYER_H
#define ANDROID_VIDEOPLAYER_PLAYER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <map>
#include <thread>
#include <list>

#include "Log.h"
#include "VideoFrame.h"
#include "AudioFrame.h"
#include "PlayerContext.h"
#include "ICodec.h"
#include "FFmpegCodec.h"
#include "output/IVideoOutput.h"
#include "output/IAudioOutput.h"
#include "StreamInfo.h"
#include "AudioConverter.h"
#include "android/audio/OboePlayer.h"
#include "Factory.h"
#include "IPlayStateListener.h"

extern "C" {
#include "FFmpeg/libavformat/avformat.h"
}

/**
 * 播放器核心。
 * ==========核心结构============
 * readStreamLoop -> (audio/video)packetQueue -> (audio/video)decodeLoop -> (audio/video)frameQueue -> syncLoop
 * packetQueue和frameQueue都使用的是限定容量的同步队列，在容量为空时阻塞pop，容量满时阻塞push。这是为了有效控制
 * 读取和解码的速度，避免速度过快导致占用内存过大。
 *
 * ==========音视频同步==========
 * packetQueue和frameQueue都使用的是限定容量的同步队列，在容量为空时阻塞pop，容量满时阻塞push。
 * 这导致容易出现死锁。
 * 例如：
 * readStreamLoop连续读取了很多audio数据。但是syncLoop一直拿不到video数据，syncLoop被阻塞在获取videoFrame上
 * -> syncLoop无法继续消费audioFrame
 * -> audioFrameQueue满
 * -> decodeAudioLoop被阻塞在发送audioFrame上，无法继续消费audioPacket
 * -> audioPacket满
 * -> readStreamLoop被阻塞在发送audioPacket上，无法继续向下读取
 * -> readStreamLoop无法获取video数据
 * -> syncLoop被阻塞在获取videoFrame上
 *
 * 为了打破上面的阻塞链，设立了两个cacheList供syncLoop使用，它们作为FIFO队列使用。syncLoop每次都是*非阻塞*地从frameQueue中拿一个
 * audioFrame/videoFrame，分别从尾部放入cacheList中，然后再从cacheList正式获取要发送的数据。这样就可以保证无论如何，
 * frameQueue都可以被顺利消费，也就不会阻塞。
 * */
class Player {

public:
    Player();
    Player(Player&) = delete;
    Player(Player&&) = delete;
    ~Player();

    void setWindow(void *window);

    bool openFile(std::string pathStr);

    void release();

    bool play();

    void pause();

    int64_t getDurationMS();

    int64_t getCurrentPtsMS();

    int getPlayState();

    bool seek(int64_t ptsMS);

    bool setScreenSize(int width, int height);

    void setPlayStateListener(IPlayStateListener *listener);

    void removePlayStateListener();

    bool isPlaying();


private:
    ICodec *videoDecoder = nullptr;
    ICodec *audioDecoder = nullptr;

    PlayerContext playerContext;

    IVideoOutput *videoOutput = nullptr;
    IAudioOutput *audioOutput = nullptr;

    AVFormatContext *formatCtx = nullptr;
    std::string filePath = "";

    int64_t durationMS = -1;

    int audioStreamIndex = -1;
    int videoStreamIndex = -1;

    std::map<int, StreamInfo> audioStreamMap;
    std::map<int, StreamInfo> videoStreamMap;

    LinkedBlockingQueue<PacketWrapper *> audioPacketQueue = LinkedBlockingQueue<PacketWrapper *>(10);
    LinkedBlockingQueue<PacketWrapper *> videoPacketQueue = LinkedBlockingQueue<PacketWrapper *>(10);

    LinkedBlockingQueue<AudioFrame *> audioFrameQueue = LinkedBlockingQueue<AudioFrame *>(10);
    LinkedBlockingQueue<VideoFrame *> videoFrameQueue = LinkedBlockingQueue<VideoFrame *>(10);

    // syncLoop内使用的cache，用来避免同步队列阻塞导致的音视频同步的死锁。
    std::list<AudioFrame *> syncAudioCacheList;
    std::list<VideoFrame *> syncVideoCacheList;

    std::thread *readStreamThread = nullptr;
    std::thread *decodeAudioThread = nullptr;
    std::thread *decodeVideoThread = nullptr;
    std::thread *syncThread = nullptr;

    std::atomic_bool stopReadFlag = false;
    std::atomic_bool stopDecodeAudioFlag = false;
    std::atomic_bool stopDecodeVideoFlag = false;
    std::atomic_bool stopSyncFlag = false;

    std::mutex readStreamMu;
    std::mutex decodeVideoMu;
    std::mutex decodeAudioMu;
    std::mutex syncMu;

    AudioFrame *unPlayedAudioFrame = nullptr;
    VideoFrame *unPlayedVideoFrame = nullptr;

    std::atomic_int64_t lastAudioPts = 0;
    std::atomic_int64_t lastVideoPts = 0;

    std::atomic_bool seekFlag = false;
    std::atomic_int64_t seekPtsMS = 0;

    void *nativeWindow = nullptr;
    int screenWidth = 1920;
    int screenHeight = 1080;

    bool enableAudio = true;
    bool enableVideo = true;

    IPlayStateListener *stateListener = nullptr;

    bool createAudioOutput();
    void releaseAudioOutput();

    bool createVideoOutput();
    void releaseVideoOutput();

    void findAvailableStreamAndDecoder(std::map<int, StreamInfo> &streams, ICodec **decoder, int *streamIndex);

    static void decodeAudioCallback(void *context);
    void decodeAudioLoop();

    static void decodeVideoCallback(void *context);
    void decodeVideoLoop();

    static void readStreamCallback(void *context);
    void readStreamLoop();

    static void syncCallback(void *context);
    void syncLoop();

    void startReadStreamThread();
    void stopReadStreamThread();

    void startDecodeVideoThread();
    void stopDecodeVideoThread();

    void startDecodeAudioThread();
    void stopDecodeAudioThread();

    void startSyncThread();
    void stopSyncThread();

    ICodec *findDecoder(AVCodecParameters *params);

    void notifyPlayState(int state);
    void notifyPlayProgress(int64_t ptsMS);
};


#endif //ANDROID_VIDEOPLAYER_PLAYER_H
