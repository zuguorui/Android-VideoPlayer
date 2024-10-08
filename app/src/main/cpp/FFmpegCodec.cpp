//
// Created by 祖国瑞 on 2022/9/19.
//

#include "FFmpegCodec.h"
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


FFmpegCodec::FFmpegCodec() {

}

FFmpegCodec::~FFmpegCodec() {
    release();
}

const char* FFmpegCodec::getName() {
    if (codec == nullptr) {
        return "Not init";
    }
    return codec->name;
}

bool FFmpegCodec::init(AVCodecParameters *params, PreferCodecType preferType, bool isEncoder) {
    _isEncoder = isEncoder;
    AVCodecID ffCodecID = AV_CODEC_ID_NONE;
    try {
        ffCodecID = AVCodecID(params->codec_id);
    } catch (...) {
        LOGE(TAG, "failed to convert %d to AVCodecID", params->codec_id);
        return false;
    }

    if (preferType == PreferCodecType::HW) {
        return findHWCodec(params, ffCodecID);
    } else if (preferType == PreferCodecType::SW) {
        return findSWCodec(params, ffCodecID);
    } else {
        if (findHWCodec(params, ffCodecID)) {
            return true;
        }
        if (findSWCodec(params, ffCodecID)) {
            return true;
        }
        return false;
    }
}

void FFmpegCodec::release() {
    LOGD(TAG, "release, codec_name = %s", codec ? codec->name : "unknown");
    if (codecCtx) {
        if (avcodec_is_open(codecCtx)) {
            avcodec_free_context(&codecCtx);
        }
        codecCtx = nullptr;
        codec = nullptr;
    }
    out_hw_pix_format = AV_PIX_FMT_NONE;
}

int FFmpegCodec::sendPacket(const AVPacket *packet) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = 0;
    ret = avcodec_send_packet(codecCtx, packet);

    return ret;
}

int FFmpegCodec::receiveFrame(AVFrame *frame) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = 0;

    ret = avcodec_receive_frame(codecCtx, frame);
    return ret;
}

void FFmpegCodec::flush() {
    if (codecCtx) {
        avcodec_flush_buffers(codecCtx);
    }
}

CodecType FFmpegCodec::getCodecType() {
    return codecType;
}

int FFmpegCodec::initHWCodec(AVCodecContext *ctx, const enum AVHWDeviceType type) {
    int err = 0;

    if ((err = av_hwdevice_ctx_create(&hwDeviceCtx, type,
                                      NULL, NULL, 0)) < 0) {
        LOGE(TAG, "Failed to setFormat specified HW device.");
        return err;
    }
    ctx->hw_device_ctx = av_buffer_ref(hwDeviceCtx);

    return err;
}

AVPixelFormat FFmpegCodec::getPixelFormat() {
    if (codecCtx == nullptr) {
        return AV_PIX_FMT_NONE;
    }
    return codecCtx->pix_fmt;
}

bool FFmpegCodec::findHWCodec(AVCodecParameters *params, AVCodecID codecId) {
    release();
    int ret;
    const char *hwDecName = getHWCodecName(codecId);
    if (hwDecName == nullptr) {
        return false;
    }


    const AVCodec *aCodec;
    if (_isEncoder) {
        aCodec = avcodec_find_encoder_by_name(hwDecName);
    } else {
        aCodec = avcodec_find_decoder_by_name(hwDecName);
    }
    if (aCodec == nullptr) {
        LOGE(TAG, "Can't find hw decoder for codec: {id = %d, hw_name = %s}", codecId, hwDecName);
        return false;
    } else {
        codec = const_cast<AVCodec *>(aCodec);
        LOGD(TAG, "find HWCodec: name = %s, long_name = %s, wrapper_name = %s", codec->name, codec->long_name, codec->wrapper_name);
        const AVCodecDescriptor *descriptor = avcodec_descriptor_get_by_name(codec->name);
        if (descriptor != nullptr) {
            LOGD(TAG, "HWCodec descriptor: name = %s, long_name = %s", descriptor->name, descriptor->long_name);
            if (descriptor->profiles != nullptr) {
                int i = 0;
                while ((descriptor->profiles + i) != nullptr && (descriptor->profiles + i)->profile != FF_PROFILE_UNKNOWN) {
                    LOGD(TAG, "HWCodec profile%d: profile = %d, name = %s", i, descriptor->profiles[i].profile, descriptor->profiles[i].name);
                    i++;
                }
            }
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

    for (int i = 0; ; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(codec, i);
        if (config == nullptr) {
            break;
        }
        LOGD(TAG, "hwConfig %d: deviceType = %d, supportHWContext = %d, pixelFormat = %d",
             i,
             config->device_type,
             (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) != 0,
             config->pix_fmt);
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
            if (initHWCodec(codecCtx, AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC) < 0) {
                LOGE(TAG, "initHWCodec failed");
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

bool FFmpegCodec::findSWCodec(AVCodecParameters *params, AVCodecID codecId) {
    release();
    int ret;
    const AVCodec *aCodec;
    if (_isEncoder) {
        aCodec = avcodec_find_encoder(codecId);
    } else {
        aCodec = avcodec_find_decoder(codecId);
    }
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

int FFmpegCodec::sendFrame(const AVFrame *frame) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = avcodec_send_frame(codecCtx, frame);
    return ret;
}

int FFmpegCodec::receivePacket(AVPacket *packet) {
    if (!codecCtx) {
        LOGE(TAG, "codecCtx is null");
        return -1001;
    }
    int ret = avcodec_receive_packet(codecCtx, packet);
    return ret;
}

bool FFmpegCodec::isEncoder() {
    return _isEncoder;
}
