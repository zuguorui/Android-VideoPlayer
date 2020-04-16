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

    if(nextVideoFrame != NULL)
    {
        delete(nextVideoFrame);
    }
}

bool VideoPlayController::openFile(const char *path) {
    if(decoder->openFile(path) == false)
    {
        LOGE("decoder open file failed");
        return false;
    }
    return true;
}

void VideoPlayController::closeFile() {
    decoder->closeInput();
}

void VideoPlayController::start() {
    audioPlayer->start();
}

void VideoPlayController::stop() {
    audioPlayer->stop();
}

void VideoPlayController::seek(int64_t posMS) {
    decoder->seekTo(posMS);
    discardAllFrame();
}

void VideoPlayController::discardAllFrame() {
    videoQueue->discardAll(NULL);
    audioQueue->discardAll(NULL);
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
    allowGetVideoFlag = false;
    videoLock->lock();
    currentPositionMS = frame->pts;
    if(nextVideoFrame == NULL)
    {
        nextVideoFrame = videoQueue->get();
    }
    while(1)
    {
        if(currentPositionMS - nextVideoFrame->pts > 10)
        {
            // if audio position is 10ms earlier than video, discard this video frame and get next
            videoQueue->putToUsed(nextVideoFrame);
            nextVideoFrame = videoQueue->get();
            continue;
        } else if(currentPositionMS - nextVideoFrame->pts < -10)
        {
            // if audio position is 10ms later than video, just let video frame wait
            allowGetVideoFlag = false;
            break;
        } else
        {
            // if audio position is 10ms close to the video, allow video refresh
            allowGetVideoFlag = true;
        }
    }

    videoLock->unlock();
    if(allowGetVideoFlag)
    {
        allowGetVideoSignal.notify_all();
    }


    return frame;
}

void VideoPlayController::putBackUsed(AudioFrame *data) {
    audioQueue->putToUsed(data);
}

VideoFrame *VideoPlayController::getVideoFrame() {
    videoLock->lock();
    while(!allowGetVideoFlag)
    {
        allowGetVideoSignal.wait(*videoLock);
    }
    VideoFrame *f = nextVideoFrame;
    nextVideoFrame = NULL;
    allowGetVideoFlag = false;
    videoLock->unlock();
    return f;
}

void VideoPlayController::putBackUsed(VideoFrame *data) {
    videoQueue->putToUsed(data);
}


