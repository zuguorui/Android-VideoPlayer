//
// Created by zu on 2024/8/10.
//

#include "FFmpegMuxer.h"
#include "Log.h"
#include "BufferBitReader.h"
#include "h264_nal.h"
#include <iostream>
#include <iomanip>
#include <sstream>



#define TAG "FFmpegMuxer"

using namespace std;

void print_hex(const uint8_t* data, int size) {
    if (!data || size <= 0) {
        return;
    }

    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');

    for (int i = 0; i < size; ++i) {
        oss << std::setw(2) << static_cast<int>(data[i]);
        if (i != size - 1) {
            oss << " ";
        }
    }

    string s = oss.str();
    LOGD(TAG, "%s", s.c_str());
}

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


void FFmpegMuxer::setCSD(uint8_t *buffer, int size, int streamIndex) {
    LOGD(TAG, "setCSD for stream %d", streamIndex);
    if (streamIndex >= streamParameters.size()) {
        throw "streamIndex out of range";
    }
    print_hex(buffer, size);
    AVCodecParameters *parameters = streamParameters[streamIndex];
    if (parameters->extradata) {
        av_free(parameters->extradata);
    }
    parameters->extradata = (uint8_t *)av_malloc(size);
    memcpy(parameters->extradata, buffer, size);
    parameters->extradata_size = size;

    // 如果已经启动了，为了应对中途CSD可能变化的情况，这里也同步更新一下formatContext里的信息
    if (formatContext && streamIndex < formatContext->nb_streams) {
        unique_lock<recursive_mutex> lock(sendDataMu);

        AVStream *stream = formatContext->streams[streamIndex];
        parameters = stream->codecpar;
        if (parameters->extradata) {
            av_free(parameters->extradata);
        }
        parameters->extradata = (uint8_t *)av_malloc(size);
        memcpy(parameters->extradata, buffer, size);
        parameters->extradata_size = size;
    }
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
        if (stream->codecpar->extradata != nullptr) {
            LOGI(TAG, "stream %d has extra data", i);
        } else {
            LOGE(TAG, "stream %d miss extra data", i);
        }
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

void FFmpegMuxer::sendData(uint8_t *data, int size, int pts, bool keyFrame, int streamIndex) {
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
    tempPacket->flags = 0;
    // 如果是I帧，带上标志。这样ffmpeg的rtmp会在该帧附加SPS/PPS
    if (keyFrame) {
        tempPacket->flags |= AV_PKT_FLAG_KEY;
    }
    sendPacket(tempPacket);

//    BufferBitReader reader(data, size);
//    NAL *nal = nullptr;
//    LOGI(TAG, "send data begin, data size = %d, pts = %d, start code = %02X %02X %02X %02X", size, pts, data[0], data[1], data[2], data[3]);
//    while ((nal = parse_nal(reader)) != nullptr) {
//        LOGI(TAG, "nal_unit_type = %d", nal->nal_unit_type);
//    }
//    LOGI(TAG, "send data end");
}

bool FFmpegMuxer::isStarted() {
    return formatContext != nullptr;
}


