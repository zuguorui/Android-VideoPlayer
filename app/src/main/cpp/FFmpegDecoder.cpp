//
// Created by 祖国瑞 on 2022/9/19.
//

#include "FFmpegDecoder.h"
#include "Log.h"

static const char* TAG = "FFmpegDecoder";

using namespace std;

static AVPixelFormat out_hw_pix_format = AV_PIX_FMT_NONE;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == out_hw_pix_format)
            return *p;
    }

    LOGE(TAG, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}


FFmpegDecoder::FFmpegDecoder() {

}

FFmpegDecoder::~FFmpegDecoder() {
    release();
}

const char* FFmpegDecoder::getName() {
    if (codec == nullptr) {
        return "Not init";
    }
    return codec->name;
}

bool FFmpegDecoder::init(AVCodecParameters *params, PreferCodecType preferType) {
    int ret;
    AVCodecID ffCodecID = AV_CODEC_ID_NONE;
    try {
        ffCodecID = AVCodecID(params->codec_id);
    } catch (...) {
        LOGE(TAG, "failed to convert %d to AVCodecID", params->codec_id);
        return false;
    }

    // 先找硬件解码器，不行再找软件解码器
    const char *hwDecName = getHWDecName(ffCodecID);
    if (hwDecName != nullptr) {
        const AVCodec * aCodec = avcodec_find_decoder_by_name(hwDecName);
        if (aCodec == nullptr) {
            LOGE(TAG, "Can't find hw decoder for codec: {id = %d, hw_name = %s}", ffCodecID, hwDecName);
        } else {
            codec = const_cast<AVCodec *>(aCodec);
        }
    }

    if (codec == nullptr) {
        const AVCodec * aCodec = avcodec_find_decoder(ffCodecID);
        if (aCodec == nullptr) {
            LOGE(TAG, "Can't find decoder for codecID %d", ffCodecID);
            return false;
        }
        codec = const_cast<AVCodec *>(aCodec);
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGE(TAG, "failed to alloc codec context");
        return false;
    }

    ret = avcodec_parameters_to_context(codecCtx, params);
    if (ret < 0) {
        LOGE(TAG, "copy decoder params failed, err = %d", ret);
        return false;
    }
    bool useHWDecoder = false;
    for (int i = 0;;i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if (config == nullptr) {
            LOGE(TAG, "%s hw config is null", codec->name);
            break;
        }
        if ((config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) &&
            config->device_type == AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC) {
            // 该解码器支持硬件解码
            out_hw_pix_format = config->pix_fmt;
            hwPixFormat = config->pix_fmt;
            codecCtx->get_format = get_hw_format;
            if (initHWDecoder(codecCtx, AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC) < 0) {
                LOGE(TAG, "initHWDecoder failed");
            } else {
                useHWDecoder = true;
                break;
            }
        }
    }

    ret = avcodec_open2(codecCtx, codec, nullptr);
    if (ret < 0) {
        char buf[100];
        av_make_error_string(buf, 100, ret);
        LOGE(TAG, "open codec failed for %s, err = %s", codec->name, buf);
        return false;
    }
    if (useHWDecoder) {
        codecType = CodecType::HW;
        LOGD(TAG, "use HW decoder for %s", codec->name);
    } else {
        codecType = CodecType::SW;
        LOGD(TAG, "use SW decoder for %s", codec->name);
    }
    return true;
}

void FFmpegDecoder::release() {
    if (codecCtx) {
        avcodec_free_context(&codecCtx);
        codecCtx = nullptr;
        codec = nullptr;
    }
}

int FFmpegDecoder::sendPacket(const AVPacket *packet) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = 0;
    ret = avcodec_send_packet(codecCtx, packet);

    return ret;
}

int FFmpegDecoder::receiveFrame(AVFrame *frame) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = 0;

    ret = avcodec_receive_frame(codecCtx, frame);
    return ret;
}

void FFmpegDecoder::flush() {
    if (codecCtx) {
        avcodec_flush_buffers(codecCtx);
    }
}

CodecType FFmpegDecoder::getCodecType() {
    return codecType;
}

int FFmpegDecoder::initHWDecoder(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hwDeviceCtx, type,
                                      NULL, NULL, 0)) < 0) {
        LOGE(TAG, "Failed to setFormat specified HW device.");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hwDeviceCtx);

    return err;
}

AVPixelFormat FFmpegDecoder::getPixelFormat() {
    if (codecCtx == nullptr) {
        return AV_PIX_FMT_NONE;
    }
    return codecCtx->pix_fmt;
}

bool FFmpegDecoder::findHWDecoder(AVCodecParameters *params, AVCodecID codecId) {
    int ret;
    const char *hwDecName = getHWDecName(codecId);
    if (hwDecName != nullptr) {
        const AVCodec * aCodec = avcodec_find_decoder_by_name(hwDecName);
        if (aCodec == nullptr) {
            LOGE(TAG, "Can't find hw decoder for codec: {id = %d, hw_name = %s}", codecId, hwDecName);
            return false;
        } else {
            codec = const_cast<AVCodec *>(aCodec);
        }
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGE(TAG, "failed to alloc codec context");
        return false;
    }

    ret = avcodec_parameters_to_context(codecCtx, params);
    if (ret < 0) {
        LOGE(TAG, "copy decoder params failed, err = %d", ret);
        return false;
    }

    for (int i = 0;;i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if (config == nullptr) {
            LOGE(TAG, "%s hw config is null", codec->name);
            break;
        }
        if ((config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) &&
            config->device_type == AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC) {
            // 该解码器支持硬件解码
            out_hw_pix_format = config->pix_fmt;
            hwPixFormat = config->pix_fmt;
            codecCtx->get_format = get_hw_format;
            if (initHWDecoder(codecCtx, AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC) < 0) {
                LOGE(TAG, "initHWDecoder failed");
                return false;
            } else {
                break;
            }
        }
    }

    ret = avcodec_open2(codecCtx, codec, nullptr);

    if (ret < 0) {
        char buf[100];
        av_make_error_string(buf, 100, ret);
        LOGE(TAG, "open codec failed for %s, err = %s", codec->name, buf);
        return false;
    }
    codecType = CodecType::HW;
    return true;
}

bool FFmpegDecoder::findSWDecoder(AVCodecParameters *params, AVCodecID codecId) {
    int ret;
    const AVCodec * aCodec = avcodec_find_decoder(codecId);
    if (aCodec == nullptr) {
        LOGE(TAG, "Can't find decoder for codecID %d", codecId);
        return false;
    }
    codec = const_cast<AVCodec *>(aCodec);

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        LOGE(TAG, "failed to alloc codec context");
        return false;
    }

    ret = avcodec_parameters_to_context(codecCtx, params);
    if (ret < 0) {
        LOGE(TAG, "copy decoder params failed, err = %d", ret);
        return false;
    }

    ret = avcodec_open2(codecCtx, codec, nullptr);
    if (ret < 0) {
        char buf[100];
        av_make_error_string(buf, 100, ret);
        LOGE(TAG, "open codec failed for %s, err = %s", codec->name, buf);
        return false;
    }
    codecType = CodecType::SW;
    return true;
}
