//
// Created by zu on 2024/8/10.
//

#include "FFmpegMuxer.h"
#include "Log.h"

#define TAG "FFmpegMuxer"

using namespace std;

FFmpegMuxer::FFmpegMuxer() {

}

FFmpegMuxer::~FFmpegMuxer() {

}

void FFmpegMuxer::setOutput(AVIOContext *ioCtx) {
    if (isStarted()) {
        return;
    }
    ioContext = ioCtx;
}

void FFmpegMuxer::setOutputFormat(const char *fmt) {
    if (isStarted()) {
        return;
    }
    if (fmt == nullptr) {
        return;
    }
    if (outputFormat) {
        free(outputFormat);
    }
    outputFormat = (char *) malloc(strlen(fmt) + 1);
    strcpy(outputFormat, fmt);
}

int FFmpegMuxer::addStream(AVCodecParameters *parameters) {
    streamParameters.push_back(parameters);
    return streamParameters.size() - 1;
}

bool FFmpegMuxer::start() {
    auto releaseResource = [&]() {
        if (formatContext) {
            avformat_free_context(formatContext);
            formatContext = nullptr;
        }
    };
    if (ioContext == nullptr) {
        return false;
    }

    if (outputFormat == nullptr) {
        return false;
    }

    if (avformat_alloc_output_context2(&formatContext, nullptr, outputFormat, nullptr) < 0) {
        LOGE(TAG, "alloc formatContext failed");
        releaseResource();
        return false;
    }
    formatContext->pb = ioContext;
    formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

    for (int i = 0; i < streamParameters.size(); i++) {
        AVStream *stream = avformat_new_stream(formatContext, nullptr);
        if (!stream) {
            releaseResource();
            LOGE(TAG, "add stream failed");
            return false;
        }
        stream->index = i;
        stream->id = i;
        AVCodecParameters *parameters = streamParameters[i];
        avcodec_parameters_copy(stream->codecpar, parameters);
    }

    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        formatContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    avformat_write_header(formatContext, nullptr);
    return true;
}

void FFmpegMuxer::stop() {
    if (formatContext == nullptr) {
        return;
    }
    av_interleaved_write_frame(formatContext, nullptr);
    av_write_trailer(formatContext);
    avformat_free_context(formatContext);
    av_free(formatContext);
    formatContext = nullptr;
    if (tempPacket) {
        av_packet_free(&tempPacket);
        tempPacket = nullptr;
    }
    for (auto *p : streamParameters) {
        avcodec_parameters_free(&p);
    }
    streamParameters.clear();
}

void FFmpegMuxer::sendPacket(AVPacket *packet) {
    unique_lock<recursive_mutex> lock(sendDataMu);
    if (formatContext == nullptr) {
        return;
    }

    AVStream *stream = formatContext->streams[packet->stream_index];
    int64_t pts = packet->pts;
    pts = av_rescale_q(pts, AV_TIME_BASE_Q, stream->time_base);
    packet->pts = pts;
    packet->dts = pts;
//    pts = (int64_t)(pts * 1000 * av_q2d(stream->time_base));
//    LOGD(TAG, "sendPacket: mediaType: %s, pts: %ld s", av_get_media_type_string(stream->codecpar->codec_type), pts);
    av_interleaved_write_frame(formatContext, packet);
}

void FFmpegMuxer::sendData(uint8_t *data, int size, int pts, int streamIndex) {
    unique_lock<recursive_mutex> lock(sendDataMu);
    if (size <= 0) {
        return;
    }
    if (tempPacket == nullptr) {
        tempPacket = av_packet_alloc();
        tempPacket->time_base = AV_TIME_BASE_Q;
    }

    tempPacket->data = (uint8_t *) malloc(size);
    memcpy(tempPacket->data, data, size);
    tempPacket->size = size;
    tempPacket->stream_index = streamIndex;
    tempPacket->pts = pts;
    sendPacket(tempPacket);
}

bool FFmpegMuxer::isStarted() {
    return formatContext != nullptr;
}


