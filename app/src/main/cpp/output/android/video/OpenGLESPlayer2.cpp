//
// Created by incus on 2020-04-21.
//

#include "OpenGLESPlayer2.h"
#include "Util.h"

#include "Log.h"

#define TAG "OpenGLESPlayer2"

using namespace std;

OpenGLESPlayer2::OpenGLESPlayer2(PlayerContext *playerContext): IVideoOutput(playerContext) {

}

OpenGLESPlayer2::~OpenGLESPlayer2() {

}

bool OpenGLESPlayer2::create(void *surface) {
    LOGD(TAG, "create");
    this->window = (ANativeWindow *)surface;
    renderThread = new thread(renderCallback, this);
    messageQueue.push(RenderMessage::SET_WINDOW);
    return true;
}

void OpenGLESPlayer2::release() {
    messageQueue.push(RenderMessage::EXIT);
    if(renderThread != nullptr && renderThread->joinable())
    {
        renderThread->join();
    }

}


void OpenGLESPlayer2::renderCallback(void *self) {
    ((OpenGLESPlayer2 *)self)->renderLoop();
}


void OpenGLESPlayer2::setScreenSize(int32_t width, int32_t height) {
    LOGD(TAG, "setScreenSize");
    this->screenWidth = width;
    this->screenHeight = height;
    messageQueue.push(RenderMessage::SET_SCREEN_SIZE);
}

void OpenGLESPlayer2::renderLoop() {
    LOGD(TAG, "renderLoop: start");
    optional<RenderMessage> messageOpt;
    RenderMessage message;
    bool exitFlag = false;
    while(!exitFlag) {
        messageOpt = messageQueue.pop();
        if (!messageOpt.has_value()) {
            LOGE(TAG, "renderLoop: messageOpt is null");
            continue;
        }
        message = messageOpt.value();
        //LOGD(TAG, "renderLoop: messsage = %d", message);
        switch (message) {
            case SET_WINDOW:
                if (!render.setWindow(window)) {
                    LOGE(TAG, "render setWindow failed");
                }
                if (!render.isEGLReady()) {
                    LOGE(TAG, "after setWindow, EGL is not ready");
                }
                break;
            case SET_SCREEN_SIZE:
                render.setScreenSize(screenWidth, screenHeight);
                break;
            case REFRESH: {
                optional<VideoFrame *> frameOpt = frameQueue.pop();
                if (frameOpt.has_value()) {
                    VideoFrame *frame = frameOpt.value();
                    if (render.isReady()) {
                        int64_t startTime = getSystemClockCurrentMilliseconds();
                        render.refresh(frame);
                        LOGD(TAG, "renderLoop: render.refresh cost %ld ms", getSystemClockCurrentMilliseconds() - startTime);
                    } else {
                        LOGE(TAG, "render is not ready");
                    }
                    playerCtx->recycleVideoFrame(frame);
                }
                break;
            }
            case SET_SRC_FORMAT:
                if (!render.setFormat(format, colorSpace, isHDR)) {
                    LOGE(TAG, "render.setFormat failed");
                }
                break;
            case EXIT:
                exitFlag = true;
                break;
            case SET_SIZE_MODE:
                render.setSizeMode(sizeMode);
                break;
            default:
                break;
        }

    }
    render.release();
}



void OpenGLESPlayer2::write(VideoFrame* frame) {
    LOGD(TAG, "write, pts = %lld, frameQueue.size = %ld", frame->pts, frameQueue.getSize());
    frameQueue.push(frame);
    messageQueue.push(RenderMessage::REFRESH);
}

bool OpenGLESPlayer2::setFormat(AVPixelFormat pixelFormat, AVColorSpace colorSpace, bool isHDR) {
    LOGD(TAG, "setFormat");
    this->format = pixelFormat;
    this->colorSpace = colorSpace;
    this->isHDR = isHDR;
    // 首先需要确保建立了EGL环境，才可以使用shader等。
    messageQueue.push(RenderMessage::SET_SRC_FORMAT);
    return true;
}

bool OpenGLESPlayer2::isReady() {
    return render.isReady();
}

void OpenGLESPlayer2::setSizeMode(SizeMode mode) {
    sizeMode = mode;
    messageQueue.push(RenderMessage::SET_SIZE_MODE);
}
