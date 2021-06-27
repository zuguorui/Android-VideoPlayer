//
// Created by 祖国瑞 on 2020-04-12.
//

#include "VideoFileDecoder.h"
#include <stdint.h>
#include <android/log.h>
#include <chrono>
#include "AACUtil.h"

using namespace std;

#define MODULE_NAME  "VideoFileDecoder"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

// For ffmpeg log
static void log_callback(void *ctx, int level, const char *fmt, va_list args)
{
    if(level == AV_LOG_ERROR)
    {
        __android_log_print(ANDROID_LOG_DEBUG, "FFmpeg", fmt, args);
    }else{
        __android_log_print(ANDROID_LOG_ERROR, "FFmpeg", fmt, args);
    }
}



VideoFileDecoder::VideoFileDecoder() {
    av_register_all();
    av_log_set_callback(log_callback);
    LOGD("constructor");
    audioPacketQueue = new BlockRecyclerQueue<AVPacket *>();
    videoPacketQueue = new BlockRecyclerQueue<AVPacket *>(20);
}

VideoFileDecoder::~VideoFileDecoder() {
    closeInput();
    delete(audioPacketQueue);
    delete(videoPacketQueue);
}

int64_t VideoFileDecoder::getDuration() {
    return duration;
}

void VideoFileDecoder::recyclePackets() {
    if(audioPacketQueue != NULL)
    {
        AVPacket *p;
        while((p = audioPacketQueue->get(false)) != NULL)
        {
            av_packet_unref(p);
            av_packet_free(&p);
        }
        while((p = audioPacketQueue->getUsed()) != NULL)
        {
            av_packet_free(&p);
        }
    }

    if(videoPacketQueue != NULL)
    {
        AVPacket *p;
        while((p = videoPacketQueue->get(false)) != NULL)
        {
            av_packet_unref(p);
            av_packet_free(&p);
        }
        while((p = videoPacketQueue->getUsed()) != NULL)
        {
            av_packet_free(&p);
        }
    }
}

bool VideoFileDecoder::openFile(const char *inputFile) {

    duration = -1;
    if(dataReceiver == NULL)
    {
        LOGE("MediaDataReceiver is NULL when start decode");
        return false;
    }
    closeInput();
    stopDecodeFlag = false;
    seekReq = false;
    if(!initComponents(inputFile))
    {
        LOGE("init components error");
        resetComponents();
        return false;
    }
    readThread = new thread(readThreadCallback, this);
    if(hasAudio())
    {
        audioDecodeThread = new thread(audioThreadCallback, this);
    }
    if(hasVideo())
    {
        videoDecodeThread = new thread(videoThreadCallback, this);
    }
    if(!hasAudio() && !hasVideo())
    {
        return false;
    }
    return true;
}

bool VideoFileDecoder::hasVideo() {
    return videoIndex != -1 && videoStream != NULL;
}

bool VideoFileDecoder::hasAudio() {
    return audioIndex != -1 && audioStream != NULL;
}

bool VideoFileDecoder::initComponents(const char *path) {
    LOGD("initComponents");
    int err = 0;
    char errbuf[256];
    formatCtx = avformat_alloc_context();
    if (!formatCtx)
    {
        LOGE("Can't allocate context\n");
        return false;
    }

    err = avformat_open_input(&formatCtx, path, NULL, NULL);
    if (err < 0)
    {
        LOGE("Can't open input file, err = %s\n", av_make_error_string(errbuf, 256, err));
        return false;
    }

    err = avformat_find_stream_info(formatCtx, NULL);
    if(err < 0)
    {
        LOGE("Error when find stream info\n");
        return false;
    }

    audioIndex = -1;
    videoIndex = -1;

    for(int i = 0; i < formatCtx->nb_streams; i++)
    {
        AVStream *stream = formatCtx->streams[i];
        AVMediaType type = stream->codecpar->codec_type;
        if(type == AVMEDIA_TYPE_AUDIO)
        {
            audioIndex = i;
            audioStream = stream;
        }
        else if(type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            videoStream = stream;
        }
    }

    if(audioIndex == -1 && videoIndex == -1)
    {
        LOGE("Neither audio stream or video stream found in this file");
        return false;
    }

    if(audioIndex != -1)
    {
        av_dump_format(formatCtx, audioIndex, NULL, 0);
    }else{
        LOGE("audio not found in this file");
    }

    if(videoIndex != -1)
    {
        av_dump_format(formatCtx, videoIndex, NULL, 0);
    } else
    {
        LOGE("video not found in this file");
    }

    // find audio stream, prepare audio decode components
    if(audioIndex != -1)
    {
        audioCodec = avcodec_find_decoder(audioStream->codecpar->codec_id);

        if (audioCodec == NULL)
        {
            LOGE("can not find audio codec for %d", audioStream->codecpar->codec_id);
            return false;
        }
        audioCodecCtx = avcodec_alloc_context3(audioCodec);
        if(audioCodecCtx == NULL)
        {
            LOGE("can not alloc audioCodecCtx");
            return false;
        }

        if (avcodec_parameters_to_context(audioCodecCtx, audioStream->codecpar) < 0)
        {
            LOGE("Error when copy params to codec context");
            return false;
        }
        if (avcodec_open2(audioCodecCtx, audioCodec, NULL) < 0)
        {
            LOGE("Error when open codec\n");
            return false;
        }

        int32_t in_sample_rate = audioCodecCtx->sample_rate;
        int32_t in_channels = audioCodecCtx->channels;
        uint64_t in_channel_layout = audioCodecCtx->channel_layout;
        AVSampleFormat in_sample_fmt = audioCodecCtx->sample_fmt;

        const int32_t out_sample_rate = AUDIO_SAMPLE_RATE;
        const int32_t out_channels = 2;
        const AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
        const uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;

        if(in_channel_layout == 0)
        {
            LOGE("in_channel_layout == 0, now we compute it by channel count");
            in_channel_layout = av_get_default_channel_layout(in_channels);
        }

        LOGE("Convert data format:\n");
        LOGE("    in_sample_rate: %d\n", in_sample_rate);
        LOGE("    in_sample_fmt: %d\n", in_sample_fmt);
        LOGE("    in_channels: %d\n", in_channels);
        LOGE("    in_channel_layout: %ld\n", in_channel_layout);

        audioSwrCtx = swr_alloc();
        audioSwrCtx = swr_alloc_set_opts(audioSwrCtx, out_channel_layout, out_sample_fmt, out_sample_rate, in_channel_layout, in_sample_fmt, in_sample_rate, 0, NULL);

        err = swr_init(audioSwrCtx);

        if(err != 0)
        {
            LOGE("audio swr init failed, err = %d", err);
        }


    }

    if(videoIndex != -1)
    {
        videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);

        if (videoCodec == NULL)
        {
            LOGE("can not find video codec for %d", videoStream->codecpar->codec_id);
            return false;
        }
        videoCodecCtx = avcodec_alloc_context3(videoCodec);
        if(videoCodecCtx == NULL)
        {
            LOGE("can not alloc videoCodecCtx");
            return false;
        }

        if (avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar) < 0)
        {
            LOGE("Error when copy params to codec context");
            return false;
        }
        if (avcodec_open2(videoCodecCtx, videoCodec, NULL) < 0)
        {
            LOGE("Error when open codec\n");
            return false;
        }

        videoSwsCtx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt, videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
        if(videoSwsCtx == NULL)
        {
            LOGE("init videoSwsCtx failed");
            return false;
        }


        videoFPS = videoStream->avg_frame_rate.num * 1.0f / videoStream->avg_frame_rate.den;

        LOGD("video FPS = %f", videoFPS);

    }

    audioSampleCountLimit = 512;
//    if(videoIndex != -1)
//    {
//        // If file contains video, we need to limit the output audio data length to let video frame can be refresh in time.
//        audioSampleCountLimit = (int32_t)(AUDIO_SAMPLE_RATE / videoFPS) + 1;
//
//    } else
//    {
//        // If it only has audio, we set it as default
//        audioSampleCountLimit = 512;
//    }


    duration = formatCtx->duration / AV_TIME_BASE * 1000;
    LOGD("get duration from format context = %ld", duration);

    return true;
}

/*
 * Don't call this function if decode thread is running. I don't know why if call this
 * at init stage, initing components will crash with SIGNAL FAULT.
 * */
void VideoFileDecoder::resetComponents() {
    unique_lock<mutex> locker(componentsMu);
    if(audioSwrCtx != NULL)
    {
        swr_free(&audioSwrCtx);
        audioSwrCtx = NULL;
    }

    if(videoSwsCtx != NULL)
    {
        sws_freeContext(videoSwsCtx);
        videoSwsCtx = NULL;
    }
    if(audioCodecCtx != NULL)
    {
        avcodec_close(audioCodecCtx);
        avcodec_free_context(&audioCodecCtx);
        audioCodecCtx = NULL;
    }

    audioCodec = NULL;

    if(videoCodecCtx != NULL)
    {
        avcodec_close(videoCodecCtx);
        avcodec_free_context(&videoCodecCtx);
        videoCodecCtx = NULL;
    }

    videoCodec = NULL;

    if(formatCtx != NULL)
    {
        avformat_close_input(&formatCtx);
        avformat_free_context(formatCtx);
        formatCtx = NULL;
    }
    locker.unlock();
}

void VideoFileDecoder::closeInput() {
    LOGD("call closeInput");
    stopDecodeFlag = true;

    if(readThread != NULL)
    {
        audioPacketQueue->notifyWaitPut();
        videoPacketQueue->notifyWaitPut();
        if(readThread->joinable())
        {
            readThread->join();
        }
        delete(readThread);
        readThread = NULL;
    }
    LOGD("readThread stopped");

    if(audioDecodeThread != NULL && audioDecodeThread->joinable())
    {
        audioPacketQueue->notifyWaitGet();
        audioDecodeThread->join();
        delete(audioDecodeThread);
        audioDecodeThread = NULL;
    }
    LOGD("decodeAudioThread stopped");
    if(videoDecodeThread != NULL && videoDecodeThread->joinable())
    {
        videoPacketQueue->notifyWaitGet();
        videoDecodeThread->join();
        delete(videoDecodeThread);
        videoDecodeThread = NULL;
    }
    LOGD("decodeVideoThread stopped");


    discardAllReadPackets();
    recyclePackets();
    resetComponents();
    LOGD("exit closeInput");
}

void VideoFileDecoder::seekTo(int64_t position) {
    if(audioDecodeThread == NULL && videoDecodeThread == NULL)
    {
        LOGE("Neither audio nor video thread is NULL when seek");
        return;
    }
    seekReq = true;
    seekPosition = position;
}

void VideoFileDecoder::setDataReceiver(IMediaDataReceiver *receiver) {
    this->dataReceiver = receiver;
}

void VideoFileDecoder::removeDataReceiber(IMediaDataReceiver *receiver) {
    if(this->dataReceiver == receiver)
    {
        this->dataReceiver = NULL;
    }
}



void VideoFileDecoder::audioThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> decodeAudio();
}

void VideoFileDecoder::videoThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> decodeVideo();
}

void VideoFileDecoder::readThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> readFile();
}

AVPacket* VideoFileDecoder::getFreePacket() {
    AVPacket *packet = NULL;
    packet = audioPacketQueue->getUsed();
    if(packet == NULL)
    {
        packet = videoPacketQueue->getUsed();
    }

    if(packet == NULL)
    {
        packet = av_packet_alloc();
        av_init_packet(packet);
    }
    return packet;
}

int32_t VideoFileDecoder::getVideoHeight() {
    if(videoStream != NULL)
    {
        return videoStream->codecpar->height;
    }
    return -1;
}

int32_t VideoFileDecoder::getVideoWidth() {
    if(videoStream != NULL)
    {
        return videoStream->codecpar->width;
    }
    return -1;
}

void VideoFileDecoder::readFile() {
    if(formatCtx == NULL)
    {
        LOGE("formatCtx is NULL when start decode");
        return;
    }
    bool readFinish = false;
    while(!stopDecodeFlag && !readFinish)
    {
        if(seekReq)
        {
            discardAllReadPackets();
            seekReq = false;
            int64_t pos = (int64_t)(seekPosition / 1000 * AV_TIME_BASE);
            if(av_seek_frame(formatCtx, -1, pos, 0) < 0)
            {
                LOGE("seek audio return error");
            }

//            if(av_seek_frame(formatCtx, videoIndex, pos, 0) < 0)
//            {
//                LOGE("seek video return error");
//            }
//            AVPacket *videoFlushPacket = getFreePacket();
//            videoFlushPacket->size = 0;
//            videoPacketQueue->put(videoFlushPacket);
//
//            AVPacket *audioFlushPacket = getFreePacket();
//            audioFlushPacket->size = 0;
//            videoPacketQueue->put(audioFlushPacket);

        }
        AVPacket *packet = getFreePacket();
        if(av_read_frame(formatCtx, packet) < 0)
        {
            //can not read more, regard as EOF
            LOGD("Finished reading audio stream");
            readFinish = true;
            //set packet size 0 to let codec flush.
            packet->size = 0;
            audioPacketQueue->put(packet);
            AVPacket *packet1 = getFreePacket();
            packet1->size = 0;
            videoPacketQueue->put(packet1);
        } else
        {
            if(packet->stream_index == audioIndex)
            {
                LOGD("read a audio packet, put it to queue, audioPacketQueue.size = %d", audioPacketQueue->getSize());
                audioPacketQueue->put(packet);
                LOGD("put audio packet finished");
            }
            else if(packet->stream_index == videoIndex)
            {
                LOGD("read a video packet, put it to queue, videoPacketQueue.size = %d", videoPacketQueue->getSize());
                videoPacketQueue->put(packet);
                LOGD("put video packet finished");
            }
            else
            {
                LOGE("unknow packet stream %d", packet->stream_index);
                av_packet_unref(packet);
                audioPacketQueue->putToUsed(packet);
            }
        }

    }
    LOGD("readThread quit");

}

void VideoFileDecoder::decodeAudio() {
    audioDecodeFinished = false;

    if(audioCodecCtx == NULL)
    {
        LOGE("audioCodecCtx is NULL when start decode");
        return;
    }

    if(audioCodec == NULL)
    {
        LOGE("audioCodec is NULL when start decode");
        return;
    }
    if(audioStream == NULL)
    {
        LOGE("audioStream is NULL when start decode");
        return;
    }
    if(audioSwrCtx == NULL)
    {
        LOGE("audioSwrCtx is NULL when start decode");
        return;
    }

    AVFrame *frame = av_frame_alloc();

    int maxAudioDataSizeInByte = audioSampleCountLimit * 2 * sizeof(int16_t);

    LOGD("max audio data size in byte = %d", maxAudioDataSizeInByte);

    int err = 0;
    bool readFinished = false;
    while(!stopDecodeFlag && !readFinished)
    {

        AVPacket *packet = audioPacketQueue->get();
        LOGD("get a audio packet, audioPacketQueue.size = %d", audioPacketQueue->getSize());
        // Someone stop decode and call notifyGetWiat(), if so, we should finish decoding.
        if(packet == NULL)
        {
            readFinished = true;
            break;
        }

        err = avcodec_send_packet(audioCodecCtx, packet);
        if(err == AVERROR(EAGAIN))
        {
            // This must not happen
        } else if (err == AVERROR_EOF){
            // codec says is EOF, cause we set the packet->size = 0.
            readFinished = true;
        } else if (err != 0){
            LOGE("audio call avcodec_send_packet() returns %d\n", err);
            continue;
        } else //err == 0
        {
            //read until can not read more to ensure codec won't be full
            while(1)
            {
                err = avcodec_receive_frame(audioCodecCtx, frame);
                if(err == AVERROR(EAGAIN))
                {
                    //Can not read until send a new packet
                    break;
                } else if(err == AVERROR_EOF)
                {
                    //The codec is flushed, no more frame will be output
                    break;
                } else if (err != 0){
                    LOGE("call avcodec_send_packet() returns %d\n", err);
                    break;
                } else // err == 0
                {
                    // convert audio until there is no more data
                    AudioFrame *audioFrame = dataReceiver->getUsedAudioFrame();
                    if(audioFrame == NULL)
                    {
                        LOGD("get used AudioFrame NULL");
                        audioFrame = new AudioFrame(maxAudioDataSizeInByte);
                    }

                    audioFrame->pts = (int64_t)(frame->pts * av_q2d(audioStream->time_base) * 1000);
                    uint8_t *tempData = (uint8_t *)(audioFrame->data);
                    audioFrame->sampleCount = swr_convert(audioSwrCtx, &(tempData), audioSampleCountLimit, (const uint8_t **)frame->data, frame->nb_samples);
                    if(audioFrame->sampleCount <= 0)
                    {
                        // there is no more data, continue to read data from file
                        dataReceiver->putUsedAudioFrame(audioFrame);
                        break;
                    } else
                    {
                        dataReceiver->receiveAudioFrame(audioFrame);
                    }
                    while(1)
                    {
                        // convert audio until there is no more data
                        AudioFrame *audioFrame = dataReceiver->getUsedAudioFrame();
                        if(audioFrame == NULL)
                        {
                            audioFrame = new AudioFrame(maxAudioDataSizeInByte);
                        }
                        memset(audioFrame->data, 0, audioSampleCountLimit * 2 * sizeof(int16_t));
                        audioFrame->pts = (int64_t)(frame->pts * av_q2d(audioStream->time_base) * 1000);
                        uint8_t *tempData = (uint8_t *)(audioFrame->data);
                        audioFrame->sampleCount = swr_convert(audioSwrCtx, &(tempData), audioSampleCountLimit, NULL, 0);
                        if(audioFrame->sampleCount <= 0)
                        {
                            // there is no more data, continue to read data from file
                            dataReceiver->putUsedAudioFrame(audioFrame);
                            break;
                        } else
                        {
                            dataReceiver->receiveAudioFrame(audioFrame);
                        }
                    }
                }
            }
        }
        av_packet_unref(packet);
        audioPacketQueue->putToUsed(packet);
    }
    av_frame_free(&frame);

    audioDecodeFinished = true;
    if(videoDecodeFinished)
    {
        resetComponents();
    }
    LOGD("decodeAudioThread quit");
}

void VideoFileDecoder::decodeVideo() {
    videoDecodeFinished = false;

    if(videoCodecCtx == NULL)
    {
        LOGE("videoCodecCtx is NULL when start decode");
        return;
    }

    if(videoCodec == NULL)
    {
        LOGE("videoCodec is NULL when start decode");
        return;
    }
    if(videoStream == NULL)
    {
        LOGE("videoStream is NULL when start decode");
        return;
    }

    if(videoSwsCtx == NULL)
    {
        LOGE("videoSwsCtx is NULL when start decode");
        return;
    }





    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, videoCodecCtx->width, videoCodecCtx->height, 1);
//    int numBytes = videoCodecCtx->width * videoCodecCtx->height * 3;

    AVFrame *frame = av_frame_alloc();
    AVFrame *convertedFrame = av_frame_alloc();

    uint8_t *outBuffer = new uint8_t[numBytes];
    av_image_fill_arrays(convertedFrame->data, convertedFrame->linesize, outBuffer, AV_PIX_FMT_RGB24, videoCodecCtx->width, videoCodecCtx->height, 1);

    bool readFinish = false;
    int err = 0;
    while(!stopDecodeFlag && !readFinish)
    {
//        LOGD("start get video packet");
        AVPacket *packet = videoPacketQueue->get();
//        LOGD("get a video packet, videoPacketQueue.size = %d", videoPacketQueue->getSize());

        if(packet == NULL)
        {
            readFinish = true;
            break;
        }
        chrono::system_clock::time_point startTime = chrono::system_clock::now();
        err = avcodec_send_packet(videoCodecCtx, packet);
        if(err == 0)
        {
            chrono::system_clock::time_point endTime = chrono::system_clock::now();
            chrono::milliseconds decodeDuration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
            LOGD("decode a video frame use %ld ms", decodeDuration.count());
        }

        if(err == AVERROR(EAGAIN))
        {
            // This must not happen
            LOGE("video send packet returns EAGAIN");
        } else if (err == AVERROR_EOF){
            // codec says is EOF, cause we set the packet->size = 0.
            LOGE("video send packet returns EOF");
            readFinish = true;
        } else if (err != 0){
            LOGE("video send packet returns %d\n", err);
            continue;
        } else //err == 0
        {
            //read until can not read more to ensure codec won't be full
            while(1)
            {
//                LOGD("video start receive frame");
                err = avcodec_receive_frame(videoCodecCtx, frame);
//                LOGD("video end receive frame");


                if(err == AVERROR(EAGAIN))
                {
                    //Can not read until send a new packet
                    LOGE("video receive frame returns EAGAIN");
                    break;
                } else if(err == AVERROR_EOF)
                {
                    //The codec is flushed, no more frame will be output
                    LOGE("video receive frame returns EOF");
                    break;
                } else if (err != 0){
                    LOGE("video receive frame returns %d\n", err);
                    break;
                } else // err == 0
                {
                    LOGE("video receive frame returns %d\n", err);
                    // convert audio until there is no more data
                    VideoFrame *videoFrame = dataReceiver->getUsedVideoFrame();
                    if(videoFrame == NULL)
                    {
                        LOGE("get used video frame NULL");
                        videoFrame = new VideoFrame(numBytes);
                        videoFrame->width = videoCodecCtx->width;
                        videoFrame->height = videoCodecCtx->height;
                    }

                    videoFrame->pts = (int64_t)(frame->pts * av_q2d(videoStream->time_base) * 1000);
                    err = sws_scale(videoSwsCtx, (const uint8_t* const *)frame->data, (const int*)frame->linesize, 0, videoCodecCtx->height, convertedFrame->data, convertedFrame->linesize);
                    if(err > 0)
                    {
                        memcpy(videoFrame->data, convertedFrame->data[0], numBytes);
                        LOGD("put a video frame to receiver");
                        dataReceiver->receiveVideoFrame(videoFrame);
                    }
                    else{
                        LOGE("sws_scale return %d", err);
                        dataReceiver->putUsedVideoFrame(videoFrame);
                    }

                }
            }

        }
        av_packet_unref(packet);
        videoPacketQueue->putToUsed(packet);
    }
    free(outBuffer);
    videoDecodeFinished = true;
    if(audioDecodeFinished)
    {
        resetComponents();
    }
    LOGD("decodeVideoThread quit");
}

void VideoFileDecoder::discardAllReadPackets() {
    videoPacketQueue->discardAll(av_packet_unref);
    audioPacketQueue->discardAll(av_packet_unref);

}