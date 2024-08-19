//
// Created by zu on 2024/8/17.
//

#include "RtmpPusher.h"
#include "Log.h"

#define TAG "RtmpPusher"

using namespace std;


RtmpPusher::RtmpPusher() {

}

RtmpPusher::~RtmpPusher() {
    stop();
    if (url) {
        free(url);
        url = nullptr;
    }
}

void RtmpPusher::setUrl(const char *pUrl) {
    if (!pUrl) {
        return;
    }
    if (url) {
        free(url);
    }
    url = (char *) malloc(strlen(pUrl) + 1);
    strcpy(url, pUrl);
}

void RtmpPusher::setOutputFormat(const char *fmt) {
    muxer.setOutputFormat(fmt);
}

int RtmpPusher::addStream(AVCodecParameters *parameters) {
    return muxer.addStream(parameters);
}

void RtmpPusher::setCSD(uint8_t *buffer, int size, int streamIndex) {
    muxer.setCSD(buffer, size, streamIndex);
}

void RtmpPusher::sendData(uint8_t *data, int size, int64_t pts, bool keyFrame, int streamIndex) {
    muxer.sendData(data, size, pts, keyFrame, streamIndex);
}


bool RtmpPusher::start() {
    LOGD(TAG, "open, url = %s", url);
    if (!initRtmp()) {
        return false;
    }
    uint8_t *buffer = (uint8_t *)av_malloc(BUFFER_SIZE);
    ioCtx = avio_alloc_context(buffer, BUFFER_SIZE, 1, this, nullptr, writeCallback, nullptr);
    muxer.setOutput(ioCtx);
    if (!muxer.start()) {
        return false;
    }
    return true;
}

void RtmpPusher::stop() {
    muxer.stop();
    // 在AVIOContext中，opaque指向的实际应该是URLContext。但是在FFmpeg中这个结构体不对外开放。
    // 这里我们传入了自己的RtmpPusher，会导致avio_close出现异常。因此这里需要先将其置空。
    ioCtx->opaque = nullptr;
    avio_closep(&ioCtx);
    releaseRtmp();
}

bool RtmpPusher::initRtmp() {
    if (url == nullptr || rtmp) {
        return false;
    }
    auto releaseSource = [&]() {
        if (rtmp) {
            RTMP_Free(rtmp);
            rtmp = nullptr;
        }
    };
    rtmp = RTMP_Alloc();
    RTMP_LogSetLevel(RTMP_LOGDEBUG);
    RTMP_LogSetCallback(rtmpLogCallback);
    RTMP_Init(rtmp);
    if (!RTMP_SetupURL(rtmp, url)) {
        LOGE(TAG, "set url false");
        releaseSource();
        return false;
    }

    RTMP_EnableWrite(rtmp);
    if (!RTMP_Connect(rtmp, nullptr)) {
        LOGE(TAG, "connect failed");
        releaseSource();
        return false;
    }
    if (!RTMP_ConnectStream(rtmp, 0)) {
        LOGE(TAG, "connect stream failed");
        releaseSource();
        return false;
    }

    return true;
}


void RtmpPusher::releaseRtmp() {
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = nullptr;
    }
}

int RtmpPusher::writeCallback(void *opaque, uint8_t *buf, int bufSize) {
    RtmpPusher *context = (RtmpPusher *)opaque;
    if (!context->rtmp) {
        return 0;
    }
    int ret = RTMP_Write(context->rtmp, (const char*)buf, bufSize);
    if (!ret) {
        LOGE(TAG, "write failed, ret = %d", ret);
    }
    return ret;
}



