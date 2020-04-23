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
    audioQueue = new BlockRecyclerQueue<AudioFrame *>(20);
    videoQueue = new BlockRecyclerQueue<VideoFrame *>(20);

    audioPlayer = new OpenSLESPlayer();

    if(!audioPlayer->create())
    {
        LOGE("audio player create failed");
    } else
    {
        audioPlayer->setAudioFrameProvider(this);
    }

    videoPlayer = new OpenGLESPlayer2();
    if(!videoPlayer->create())
    {
        LOGE("video player create failed");
    } else
    {
        videoPlayer->setVideoFrameProvider(this);
    }

    exitFlag = false;
//    imageRefreshThread = new thread(refreshThreadCallback, this);

}

VideoPlayController::~VideoPlayController() {

    videoQueue->notifyWaitGet();
    exitFlag = true;
    if(imageRefreshThread != NULL && imageRefreshThread->joinable())
    {
        imageRefreshThread->join();
        delete(imageRefreshThread);
    }

    videoQueue->notifyWaitPut();
    audioQueue->notifyWaitPut();
    if(decoder != NULL)
    {
        decoder->closeInput();
        delete(decoder);
        decoder = NULL;
    }

    audioQueue->notifyWaitGet();
    if(audioPlayer != NULL)
    {
        audioPlayer->stop();
        audioPlayer->release();
        delete(audioPlayer);
        audioPlayer = NULL;
    }
    if(videoPlayer != NULL)
    {
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

void* VideoPlayController::refreshThreadCallback(void *self) {
    ((VideoPlayController *)self)->imageRefreshLoop();
}

void VideoPlayController::imageRefreshLoop() {
    while(!exitFlag)
    {
        VideoFrame *f = videoQueue->get();
        int64_t pos = currentPositionMS;
        if(f->pts - pos < -50)
        {
            //video too slow, discard it
            LOGD("video too slow, current pos = %ld, video.pts = %ld", pos, f->pts);
            videoQueue->putToUsed(f);
        } else if(f->pts - pos > 50)
        {
            //video too fast, wait
            LOGD("video too fast, wait, current pos = %ld, video.pts = %ld", pos, f->pts);
            unique_lock<mutex> locker(currentPosMu);
            while(f->pts - currentPositionMS > 50)
            {
                updateCurrentPosSignal.wait(locker);
            }
            locker.unlock();
            if(f->pts - pos < -50)
            {
                //discard
                LOGD("after wait pos update, discard video");
                videoQueue->putToUsed(f);
            } else
            {
                //use it
                LOGD("after wait pos update, use video");
                nextVideoFrame = f;
                videoPlayer->refresh();
            }
        } else
        {
            nextVideoFrame = f;
            videoPlayer->refresh();
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
    LOGD("get a audio frame, pts = %ld", currentPositionMS);
//    updateCurrentPosSignal.notify_all();

    unique_lock<mutex> locker(videoMu);
    while(1)
    {

        if(nextVideoFrame == NULL)
        {
            nextVideoFrame = videoQueue->get();
            if(nextVideoFrame == NULL)
            {
                ///if get a NULL videoFrame, means this controller will terminate
                break;
            }
            LOGD("get a video frame, pts = %ld", nextVideoFrame->pts);
        }

        LOGD("this video frame, pts = %ld", nextVideoFrame->pts);

        if(nextVideoFrame->pts - currentPositionMS < -50)
        {
            ///this video too late, discard it. And continue this loop until get a videoFrame is not too late
            LOGD("video too late, discard");
            videoQueue->putToUsed(nextVideoFrame);
            nextVideoFrame = NULL;
            continue;
        } else if(nextVideoFrame->pts - currentPositionMS > 50)
        {
            ///this video is still too early, wait. break this loop to let the audioFrame return. And will check this video frame at next getAudioFrame call.
            LOGD("video too early, wait");
            break;
        } else
        {
            ///it is time to refresh this videoFrame.
            if(videoPlayer->isReady())
            {
                LOGD("refresh image");
                videoPlayer->refresh();
            } else
            {
                ///videoPlayer not ready(caused window not set), discard this video frame
                LOGE("videoPlayer not prepared, discard this videoFrame");
                videoQueue->putToUsed(nextVideoFrame);
                nextVideoFrame = NULL;
            }
            break;
        }
    }
    locker.unlock();

    return frame;
}

void VideoPlayController::putBackUsed(AudioFrame *data) {
    audioQueue->putToUsed(data);
}

VideoFrame *VideoPlayController::getVideoFrame() {
    LOGD("the videoPlayer call getVideoFrame");
    unique_lock<mutex> locker(videoMu);
    VideoFrame *f = nextVideoFrame;
    nextVideoFrame = NULL;
    locker.unlock();
    return f;
}

void VideoPlayController::putBackUsed(VideoFrame *data) {
    videoQueue->putToUsed(data);
}


