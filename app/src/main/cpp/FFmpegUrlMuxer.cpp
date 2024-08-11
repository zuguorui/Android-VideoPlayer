//
// Created by zu on 2024/8/10.
//

#include "FFmpegUrlMuxer.h"
#include "Log.h"

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

void FFmpegUrlMuxer::sendData(uint8_t *data, int size, int64_t pts, int streamIndex) {
    muxer.sendData(data, size, pts, streamIndex);
}

bool FFmpegUrlMuxer::start() {
    auto releaseResource = [&]() {
        if (ioContext) {
            avio_context_free(&ioContext);
            ioContext = nullptr;
        }
    };
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
    avio_context_free(&ioContext);
    ioContext = nullptr;
}


