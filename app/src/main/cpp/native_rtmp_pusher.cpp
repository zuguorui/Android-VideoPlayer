//
// Created by zu on 2024/8/19.
//

#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include <map>

#include "RtmpPusher.h"
#include "Log.h"
#include "utils.h"

using namespace std;

#define TAG "native_rtmp_pusher"

map<int64_t, RtmpPusher *> objectMap;

RtmpPusher *getBoundObject(JNIEnv *env, jobject &thiz, int64_t &id) {
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    id = env->GetLongField(thiz, fieldID);
    return objectMap[id];
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nInit(JNIEnv *env, jobject thiz) {
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    RtmpPusher *pusher = new RtmpPusher();
    objectMap[(int64_t)pusher] = pusher;
    env->SetLongField(thiz, fieldID, (int64_t)pusher);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nRelease(JNIEnv *env, jobject thiz) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (pusher) {
        objectMap.erase(id);
        delete(pusher);
    }
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    env->SetLongField(thiz, fieldID, 0L);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nSetUrl(JNIEnv *env, jobject thiz, jstring pUrl) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return;
    }
    const char *url = env->GetStringUTFChars(pUrl, nullptr);
    pusher->setUrl(url);
    env->ReleaseStringUTFChars(pUrl, url);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nAddAudioStream(JNIEnv *env, jobject thiz,
                                                         jstring mime_type, jint sample_rate,
                                                         jint channels, jint bit_rate) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return -1;
    }

    const char *mimeType = env->GetStringUTFChars(mime_type, nullptr);
    AVCodecID codecId = get_codec_id_by_mimetype(mimeType);
    if (codecId == AVCodecID::AV_CODEC_ID_NONE) {
        LOGE(TAG, "failed to get codecID for mimetype: %s", mimeType);
    }
    env->ReleaseStringUTFChars(mime_type, mimeType);

    if (codecId == AVCodecID::AV_CODEC_ID_NONE) {
        return -1;
    }
    AVCodecParameters *parameters = avcodec_parameters_alloc();
    parameters->codec_type = AVMediaType::AVMEDIA_TYPE_AUDIO;
    parameters->codec_id = codecId;
    parameters->sample_rate = sample_rate;
    parameters->format = AVSampleFormat::AV_SAMPLE_FMT_S16;
    parameters->bit_rate = bit_rate;
    av_channel_layout_default(&parameters->ch_layout, channels);

    return pusher->addStream(parameters);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nAddVideoStream(JNIEnv *env, jobject thiz,
                                                         jstring mime_type, jdouble fps, jint width,
                                                         jint height, jint pixel_fmt, jint profile,
                                                         jint level, jint bit_rate) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return -1;
    }

    const char *mimeType = env->GetStringUTFChars(mime_type, nullptr);
    AVCodecID codecId = get_codec_id_by_mimetype(mimeType);
    if (codecId == AVCodecID::AV_CODEC_ID_NONE) {
        LOGE(TAG, "failed to get codecID for mimetype: %s", mimeType);
    }
    env->ReleaseStringUTFChars(mime_type, mimeType);

    if (codecId == AVCodecID::AV_CODEC_ID_NONE) {
        return -1;
    }
    AVCodecParameters *parameters = avcodec_parameters_alloc();
    parameters->codec_type = AVMediaType::AVMEDIA_TYPE_VIDEO;
    parameters->codec_id = codecId;
    parameters->format = AVPixelFormat::AV_PIX_FMT_NV12;
    parameters->width = width;
    parameters->height = height;
    parameters->profile = profile;
    parameters->level = level;
    parameters->bit_rate = bit_rate;

    return pusher->addStream(parameters);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nSetCSD(JNIEnv *env, jobject thiz, jobject buffer,
                                                 jint offset, jint size, jint stream_index) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return;
    }
    uint8_t *data = (uint8_t *)env->GetDirectBufferAddress(buffer);
    pusher->setCSD(data + offset, size, stream_index);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nSetOutputFormat(JNIEnv *env, jobject thiz, jstring pFmt) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return;
    }
    const char *fmt = env->GetStringUTFChars(pFmt, nullptr);
    pusher->setOutputFormat(fmt);
    env->ReleaseStringUTFChars(pFmt, fmt);
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nStart(JNIEnv *env, jobject thiz) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return false;
    }
    return pusher->start();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nStop(JNIEnv *env, jobject thiz) {
    int64_t id;
    RtmpPusher *muxer = getBoundObject(env, thiz, id);
    if (!muxer) {
        return;
    }
    muxer->stop();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_RtmpPusher_nSendData(JNIEnv *env, jobject thiz, jobject buffer,
                                                   jint offset, jint count, jlong pts,
                                                   jboolean key_frame, jint stream_index) {
    int64_t id;
    RtmpPusher *pusher = getBoundObject(env, thiz, id);
    if (!pusher) {
        return;
    }

    uint8_t *data = (uint8_t *)env->GetDirectBufferAddress(buffer);
    pusher->sendData(data + offset, count, pts, key_frame, stream_index);
}