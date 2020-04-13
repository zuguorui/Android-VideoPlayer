//
// Created by 祖国瑞 on 2020-04-12.
//

#include "VideoFileDecoder.h"
#include <stdint.h>
#include <android/log.h>
#include "AACUtil.h"


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

    audioPacketQueue = new BlockRecyclerQueue<AVPacket *>(100);
    videoPacketQueue = new BlockRecyclerQueue<AVPacket *>(5);
}

VideoFileDecoder::~VideoFileDecoder() {

}

void VideoFileDecoder::recyclePackets() {
    if(audioPacketQueue != NULL)
    {
        AVPacket *p;
        while((p = audioPacketQueue->get()) != NULL)
        {

        }
    }
}

void VideoFileDecoder::openFile(const char *inputFile) {

    if(dataReceiver == NULL)
    {
        LOGE("MediaDataReceiver is NULL when start decode");
        return;
    }
    closeInput();
    stopDecodeFlag = false;
    seekAudioReq = false;
    seekVideoReq = false;
    if(!initComponents(inputFile))
    {
        LOGE("init components error");
        resetComponents();
        return;
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
}

bool VideoFileDecoder::hasVideo() {
    return videoIndex != -1 && videoStream != NULL;
}

bool VideoFileDecoder::hasAudio() {
    return audioIndex != -1 && audioStream != NULL;
}

bool VideoFileDecoder::initComponents(const char *path) {
    int err = 0;
    formatCtx = avformat_alloc_context();
    if (!formatCtx)
    {
        LOGE("Can't allocate context\n");
        return false;
    }

    err = avformat_open_input(&formatCtx, path, NULL, NULL);
    if (err < 0)
    {
        LOGE("Can't open input file\n");
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

        const int32_t out_sample_rate = 44100;
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
            LOGE("swr init failed, err = %d", err);
        }

        // if audio is AAC, we need to deal with duration. FFmpeg can not get AAC duration correctly
        if(audioCodec->id == AV_CODEC_ID_AAC)
        {
            FILE *aacFile = fopen(path, "rb");
            if(aacFile != NULL)
            {
                duration = get_aac_duration(aacFile);
                fclose(aacFile);
            }else{
                LOGE("open aac file error");
            }

            if(duration == -1)
            {
                LOGE("get aac duration error, now we use audio stream duration, it may be wrong for aac");
                duration = (int64_t)(audioStream->duration * av_q2d(audioStream->time_base) * 1000);
            }
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

        videoFPS = videoStream->avg_frame_rate.num * 1.0f / videoStream->avg_frame_rate.den;

    }

    if(videoIndex != -1)
    {
        // If file contains video, we need to limit the output audio data length to let video frame can be refresh in time.
        audioSampleCountLimit = (int32_t)(AUDIO_SAMPLE_RATE / (1000.0f / videoFPS)) + 1;
    } else
    {
        // If it only has audio, we set it as default
        audioSampleCountLimit = 512;
    }

    return true;

}

/*
 * Don't call this function if decode thread is running. I don't know why if call this
 * at init stage, initing components will crash with SIGNAL FAULT.
 * */
void VideoFileDecoder::resetComponents() {
    if(audioSwrCtx != NULL)
    {
        swr_free(&audioSwrCtx);
        audioSwrCtx = NULL;
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
}

void VideoFileDecoder::closeInput() {
    stopDecodeFlag = true;
    if(audioDecodeThread != NULL && audioDecodeThread->joinable())
    {
        audioDecodeThread->join();
    }
    if(videoDecodeThread != NULL && videoDecodeThread->joinable())
    {
        videoDecodeThread->join();
    }
    resetComponents();
}

void VideoFileDecoder::seekTo(int64_t position) {
    if(audioDecodeThread == NULL && videoDecodeThread == NULL)
    {
        LOGE("Neither audio nor video thread is NULL when seek");
        return;
    }
    audioSeekPosition = (int64_t)(position / 1000 / av_q2d(audioStream->time_base));
    videoSeekPosition = (int64_t)(position / 1000 / av_q2d(videoStream->time_base));
    seekAudioReq = true;
    seekVideoReq = true;
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



void* VideoFileDecoder::audioThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> decodeAudio();
}

void* VideoFileDecoder::videoThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> decodeVideo();
}

void* VideoFileDecoder::readThreadCallback(void *context) {
    ((VideoFileDecoder *)context) -> readFile();
}

AVPacket* VideoFileDecoder::getFreePacket() {

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

void VideoFileDecoder::decodeAudio() {
    audioDecodeFinished = false;
    if(formatCtx == NULL)
    {
        LOGE("formatCtx is NULL when start decode");
        return;
    }
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

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    AVFrame *frame = av_frame_alloc();

    bool readFinish = false;
    int err = 0;
    while(!stopDecodeFlag && !readFinish)
    {
        if(seekAudioReq)
        {
            seekAudioReq = false;
            av_seek_frame(formatCtx, audioIndex, audioSeekPosition, 0);
        }
        if(av_read_frame(formatCtx, packet) < 0)
        {
            //can not read more, regard as EOF
            LOGD("Finished reading audio stream");
            readFinish = true;
            //set packet size 0 to let codec flush.
            packet->size = 0;
        }

        err = avcodec_send_packet(audioCodecCtx, packet);
        if(err == AVERROR(EAGAIN))
        {
            // This must not happen
        } else if (err == AVERROR_EOF){
            // codec says is EOF, cause we set the packet->size = 0.
        } else if (err != 0){
            LOGE("call avcodec_send_packet() returns %d\n", err);
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
                } else // err == 0
                {
                    while(1)
                    {
                        // convert audio until there is no more data
                        AudioFrame *audioFrame = dataReceiver->getUsedAudioFrame();
                        if(audioFrame == NULL)
                        {
                            audioFrame = new AudioFrame();
                            audioFrame->data = (int16_t *)malloc(audioSampleCountLimit * 2 * sizeof(int16_t));
                        }
                        memset(audioFrame, 0, audioSampleCountLimit * 2 * sizeof(int16_t));
                        uint8_t *tempData = (uint8_t *)audioFrame->data;
                        audioFrame->sampleCount = swr_convert(audioSwrCtx, &(tempData), audioSampleCountLimit, (const uint8_t **)frame->data, frame->nb_samples);
                        if(audioFrame->sampleCount < 0)
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

    }
    av_frame_free(&frame);
    av_free(packet);

    audioDecodeFinished = true;
    if(videoDecodeFinished)
    {
        resetComponents();
    }

}

void VideoFileDecoder::decodeVideo() {
    videoDecodeFinished = true;

    if(formatCtx == NULL)
    {
        LOGE("formatCtx is NULL when start decode");
        return;
    }
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

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(packet);

    AVFrame *frame = av_frame_alloc();

    bool readFinish = false;
    int err = 0;
    while(!stopDecodeFlag && !readFinish)
    {
        if(seekVideoReq)
        {
            seekVideoReq = false;
            av_seek_frame(formatCtx, videoIndex, videoSeekPosition, 0);
        }
        if(av_read_frame(formatCtx, packet) < 0)
        {
            //can not read more, regard as EOF
            LOGD("Finished reading video stream");
            readFinish = true;
            //set packet size 0 to let codec flush.
            packet->size = 0;
        }

        err = avcodec_send_packet(audioCodecCtx, packet);
        if(err == AVERROR(EAGAIN))
        {
            // This must not happen
        } else if (err == AVERROR_EOF){
            // codec says is EOF, cause we set the packet->size = 0.
        } else if (err != 0){
            LOGE("call avcodec_send_packet() returns %d\n", err);
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
                } else // err == 0
                {
                    while(1)
                    {
                        // convert audio until there is no more data
                        AudioFrame *audioFrame = dataReceiver->getUsedAudioFrame();
                        if(audioFrame == NULL)
                        {
                            audioFrame = new AudioFrame();
                            audioFrame->data = (int16_t *)malloc(audioSampleCountLimit * 2 * sizeof(int16_t));
                        }
                        memset(audioFrame, 0, audioSampleCountLimit * 2 * sizeof(int16_t));
                        uint8_t *tempData = (uint8_t *)audioFrame->data;
                        audioFrame->sampleCount = swr_convert(audioSwrCtx, &(tempData), audioSampleCountLimit, (const uint8_t **)frame->data, frame->nb_samples);
                        if(audioFrame->sampleCount < 0)
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
    }
}