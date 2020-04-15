//
// Created by 祖国瑞 on 2020-04-11.
//

#include "VideoPlayController.h"
#include "OpenSLESPlayer.h"
#include "OpenGLESPlayer.h"
#include <android/log.h>

#define MODULE_NAME  "VideoPlayController"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

VideoPlayController::VideoPlayController() {
    decoder = new VideoFileDecoder();
    audioLock = new unique_lock<mutex>(audioMu);
    audioLock->unlock();
    videoLock = new unique_lock<mutex>(videoMu);
    videoLock->unlock();

    audioQueue = new BlockRecyclerQueue<AudioFrame *>();
    videoQueue = new BlockRecyclerQueue<VideoFrame *>();

    audioPlayer = new OpenSLESPlayer();

    if(!audioPlayer->create())
    {
        LOGE("audio player create failed");
    } else
    {
        audioPlayer->setAudioFrameProvider(this);
    }

    videoPlayer = new OpenGLESPlayer();
    if(!videoPlayer->create())
    {
        LOGE("video player create failed");
    } else
    {
        videoPlayer->setVideoFrameProvider(this);
    }

}

VideoPlayController::~VideoPlayController() {
    if(decoder != NULL)
    {
        decoder->closeInput();
        delete(decoder);
        decoder = NULL;
    }
    if(audioPlayer != NULL)
    {
        audioPlayer->stop();
        audioPlayer->release();
        delete(audioPlayer);
        audioPlayer = NULL;
    }
    if(videoPlayer != NULL)
    {
        videoPlayer->stop();
        videoPlayer->release();
        delete(videoPlayer);
        videoPlayer = NULL;
    }
    if(audioLock != NULL)
    {
        audioLock->unlock();
        delete(audioLock);
        audioLock = NULL;
    }
    if(videoLock != NULL)
    {
        videoLock->unlock();
        delete(videoLock);
        videoLock = NULL;
    }

    if(audioQueue != NULL)
    {
        AudioFrame *frame = NULL;
        while((frame = audioQueue->get(false)) != NULL)
        {
            delete(frame);
        }
        while((frame = audioQueue->getUsed()) != NULL)
        {
            delete(frame);
        }
    }

    if(videoQueue != NULL)
    {
        VideoFrame *frame = NULL;
        while((frame = videoQueue->get(false)) != NULL)
        {
            delete(frame);
        }
        while((frame = videoQueue->getUsed()) != NULL)
        {
            delete(frame);
        }
    }
}

void VideoPlayController::receiveAudioFrame(AudioFrame *audioData) {
    audioQueue->put(audioData);
}

void VideoPlayController::receiveVideoFrame(VideoFrame *videoData) {
    videoQueue->put(videoData);
}

AudioFrame *VideoPlayController::getUsedAudioFrame() {

    return audioQueue->getUsed();
}

VideoFrame *VideoPlayController::getUsedVideoFrame() {
    return videoQueue->getUsed();
}

void VideoPlayController::putUsedAudioFrame(AudioFrame *audioData) {
    audioQueue->putToUsed(audioData);
}

void VideoPlayController::putUsedVideoFrame(VideoFrame *videoData) {
    videoQueue->putToUsed(videoData);
}

AudioFrame *VideoPlayController::getAudioFrame() {
    AudioFrame *frame = audioQueue->get();
    currentPositionMS = frame->pts;
    if(nextVideoFrame != NULL)
    {
        if(currentPositionMS - nextVideoFrame->pts > 20)
        {
            // audio is faster 20ms than video, discard this video frame
            videoQueue->putToUsed(nextVideoFrame);
            nextVideoFrame = NULL;
        } else if(currentPositionMS - nextVideoFrame->pts < -20)
        {

        } else
        {

        }
    }
}

void VideoPlayController::putBackUsed(AudioFrame *data) {

}

VideoFrame *VideoPlayController::getVideoFrame() {
    return nullptr;
}

void VideoPlayController::putBackUsed(VideoFrame *data) {

}


