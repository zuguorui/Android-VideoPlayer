//
// Created by zu on 2024/8/10.
//

#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <media/NdkMediaFormat.h>

#include "FFmpegUrlMuxer.h"
#include "utils.h"
#include "Log.h"

using namespace std;

#define TAG "native_url_muxer"

map<int64_t, FFmpegUrlMuxer *> muxerMap;

FFmpegUrlMuxer *getBoundMuxer(JNIEnv *env, jobject thiz, int64_t &id) {
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    id = env->GetLongField(thiz, fieldID);
    return muxerMap[id];
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nInit(JNIEnv *env, jobject thiz) {
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    FFmpegUrlMuxer *muxer = new FFmpegUrlMuxer();
    muxerMap[(int64_t)muxer] = muxer;
    env->SetLongField(thiz, fieldID, (int64_t)muxer);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nRelease(JNIEnv *env, jobject thiz) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (muxer) {
        muxerMap.erase(id);
        delete(muxer);
    }
    jclass clazz = env->GetObjectClass(thiz);
    jfieldID fieldID = env->GetFieldID(clazz, "nativeObjectID", "J");
    env->SetLongField(thiz, fieldID, 0L);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nSetUrl(JNIEnv *env, jobject thiz, jstring pUrl) {

    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
        return;
    }
    const char *url = env->GetStringUTFChars(pUrl, nullptr);
    muxer->setUrl(url);
    env->ReleaseStringUTFChars(pUrl, url);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nSetOutputFormat(JNIEnv *env, jobject thiz, jstring pFmt) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
        return;
    }
    const char *fmt = env->GetStringUTFChars(pFmt, nullptr);
    muxer->setOutputFormat(fmt);
    env->ReleaseStringUTFChars(pFmt, fmt);
}



extern "C"
JNIEXPORT jboolean JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nStart(JNIEnv *env, jobject thiz) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
        return false;
    }
    return muxer->start();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nStop(JNIEnv *env, jobject thiz) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
        return;
    }
    muxer->stop();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nSendData(JNIEnv *env, jobject thiz, jobject buffer,
                                                 jint offset, jint count, jlong pts,
                                                 jint stream_index) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
        return;
    }

    uint8_t *data = (uint8_t *)env->GetDirectBufferAddress(buffer);
    muxer->sendData(data + offset, count, pts, stream_index);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nAddAudioStream(JNIEnv *env, jobject thiz,
                                                             jstring mime_type, jint sample_rate,
                                                             jint channels) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
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
    av_channel_layout_default(&parameters->ch_layout, channels);

    return muxer->addStream(parameters);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_zu_videoplayer_muxer_FFmpegUrlMuxer_nAddVideoStream(JNIEnv *env, jobject thiz,
                                                             jstring mime_type, jdouble fps,
                                                             jint width, jint height,
                                                             jint pixel_fmt, jint profile,
                                                             jint level) {
    int64_t id;
    FFmpegUrlMuxer *muxer = getBoundMuxer(env, thiz, id);
    if (!muxer) {
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

    return muxer->addStream(parameters);
}