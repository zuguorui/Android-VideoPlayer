//
// Created by zu on 2024/8/10.
//

#include "FFmpegUrlMuxer.h"
#include "Log.h"

#define TAG "FFmpegUrlMuxer"

using namespace std;

FFmpegUrlMuxer::FFmpegUrlMuxer() {
    av_log_set_callback(ffmpegLogCallback);
}

void FFmpegUrlMuxer::setUrl(const char *pUrl) {
    if (!pUrl) {
        return;
    }
    if (url) {
        free(url);
    }
    url = (char *) malloc(strlen(pUrl) + 1);
    strcpy(url, pUrl);
}

void FFmpegUrlMuxer::setOutputFormat(const char *fmt) {
    muxer.setOutputFormat(fmt);
}

int FFmpegUrlMuxer::addStream(AVCodecParameters *parameters) {
    return muxer.addStream(parameters);
}

void FFmpegUrlMuxer::setCSD(uint8_t *buffer, int size, int streamIndex) {
    muxer.setCSD(buffer, size, streamIndex);
}

void FFmpegUrlMuxer::sendData(uint8_t *data, int size, int64_t pts, bool keyFrame, int streamIndex) {
    muxer.sendData(data, size, pts, keyFrame, streamIndex);
}

bool FFmpegUrlMuxer::start() {
    auto releaseResource = [&]() {

    };
    LOGD(TAG, "open, url = %s", url);
    avformat_network_init();
    if (avio_open(&ioContext, url, AVIO_FLAG_WRITE) < 0) {
        releaseResource();
        return false;
    }

    muxer.setOutput(ioContext);
    if (!muxer.start()) {
        return false;
    }
    return true;
}

void FFmpegUrlMuxer::stop() {
    muxer.stop();
    avio_closep(&ioContext);
}


