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
    LOGD("constructor");
    decoder = new VideoFileDecoder();

    decoder->setDataReceiver(this);
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
    LOGD("openFile");
    if(decoder->openFile(path) == false)
    {
        LOGE("decoder open file failed");
        return false;
    }
    return true;
}

void VideoPlayController::closeFile() {
    LOGD("closeFile");
    decoder->closeInput();
}

void VideoPlayController::start() {
    LOGD("start");
    audioPlayer->start();
}

void VideoPlayController::stop() {
    LOGD("stop");
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

void VideoPlayController::setWindow(void *window) {
    LOGD("setWindow");
    videoPlayer->setWindow(window);
}

void VideoPlayController::setSize(int width, int height) {
    LOGD("setSize");
    videoPlayer->setSize(width, height);
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
    unique_lock<mutex> videoLock = unique_lock<mutex>(videoMu);
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
            break;
        }
    }

    videoLock.unlock();
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
    unique_lock<mutex> videoLock = unique_lock<mutex>(videoMu);
    while(!allowGetVideoFlag)
    {
        allowGetVideoSignal.wait(videoLock);
    }
    VideoFrame *f = nextVideoFrame;
    nextVideoFrame = NULL;
    allowGetVideoFlag = false;
    videoLock.unlock();
    return f;
}

void VideoPlayController::putBackUsed(VideoFrame *data) {
    videoQueue->putToUsed(data);
}


