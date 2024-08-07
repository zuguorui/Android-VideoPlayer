//
// Created by 祖国瑞 on 2022/12/6.
//

#include "GLESRender.h"
#include "Log.h"
#include "utils.h"

#define TAG "GLESRender"

using namespace std;

void dumpData(const char *name, uint8_t *data, int64_t size) {
    char fullPath[50];
    sprintf(fullPath, "/data/data/com.zu.android_videoplayer/cache/%s", name);
    FILE *f = fopen(fullPath, "wb");
    fwrite(data, 1, size, f);
    fflush(f);
    fclose(f);
}


GLESRender::GLESRender() {

}

GLESRender::~GLESRender() {
    release();
}

bool GLESRender::setFormat(AVPixelFormat format, AVColorSpace colorSpace, bool isHDR) {

    LOGD(TAG, "setFormat, format = %s", av_get_pix_fmt_name(format));

    if (!eglWindow.isReady()) {
        LOGE(TAG, "eglWindow is not ready");
        return false;
    }

    eglWindow.makeCurrent();

    pixelType = get_pixel_type(format);
    pixelLayout = get_pixel_layout(format);
    this->format = format;

    IRenderLogic *newLogic = nullptr;
    switch (format) {
        case AVPixelFormat::AV_PIX_FMT_YUV420P:
            newLogic = new YUV420P_RenderLogic();
            break;
        case AVPixelFormat::AV_PIX_FMT_NV21:
            newLogic = new NV21_RenderLogic();
            break;
        case AVPixelFormat::AV_PIX_FMT_NV12:
            newLogic = new NV12_RenderLogic();
            break;
        default:
            break;
    }

    if (newLogic == nullptr) {
        LOGE(TAG, "unsupported pixel format: %s", av_get_pix_fmt_name(format));
        return false;
    }

    if (renderLogic) {
        delete(renderLogic);
    }
    renderLogic = newLogic;
    renderLogic->config(screenWidth, screenHeight, frameWidth, frameHeight, orientation, sizeMode);
    if (!renderLogic->isReady()) {
        LOGE(TAG, "renderLogic is not ready");
        return false;
    }
    return true;
}

bool GLESRender::setWindow(ANativeWindow *window) {
    LOGD(TAG, "setWindow");
    if (!eglWindow.create(window)) {
        LOGE(TAG, "setFormat eglWindow failed");
        return false;
    }
    if (!eglWindow.isReady()) {
        LOGE(TAG, "eglWindow not ready");
        return false;
    }
    return true;
}

void GLESRender::setScreenSize(int width, int height) {
    if (renderLogic == nullptr) {
        LOGE(TAG, "renderLogic is null");
        return;
    }
    renderLogic->setScreenSize(width, height);
    screenWidth = width;
    screenHeight = height;
    if (eglWindow.isReady()) {
        glViewport(0, 0, width, height);
    }
}

void GLESRender::release() {

    if (renderLogic) {
        delete(renderLogic);
        renderLogic = nullptr;
    }
    eglWindow.release();

}

void GLESRender::refresh(VideoFrame *videoFrame) {
    if (videoFrame->pixelFormat != format) {
        LOGE(TAG, "frame.format != format, frame.format = %s, format = %s", av_get_pix_fmt_name(videoFrame->pixelFormat),
             av_get_pix_fmt_name(format));
        return;
    }

    if (!renderLogic) {
        LOGE(TAG, "renderLogic is null");
        return;
    }

    eglWindow.makeCurrent();

    renderLogic->setFrameSize(videoFrame->width, videoFrame->height);
    renderLogic->setRotation(videoFrame->rotation);
    renderLogic->render(videoFrame);

    eglWindow.swapBuffer();

}

bool GLESRender::isReady() {
    return eglWindow.isReady() && renderLogic != nullptr;
}

bool GLESRender::isEGLReady() {
    return eglWindow.isReady();
}

void GLESRender::setSizeMode(SizeMode mode) {
    sizeMode = mode;
    if (!renderLogic) {
        LOGE(TAG, "renderLogic is null");
        return;
    }
    renderLogic->setSizeMode(sizeMode);
}


