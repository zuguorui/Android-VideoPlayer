//
// Created by incus-pc-2 on 2020/1/7.
//

/**
 * See blog
 * https://blog.csdn.net/u013470102/article/details/80880079
 * and
 * https://blog.csdn.net/zhangrui_fslib_org/article/details/50756640
 *
 * */

#include "AACUtil.h"
#include <stdio.h>
#include <string.h>
#include <android/log.h>
#include <sys/stat.h>

#define MODULE_NAME  "AACUtil"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__)

uint64_t convert_header_to_int64(int8_t header[ADTS_HEADER_LEN])
{
    uint64_t result = 0;
    for(int i = 0; i < ADTS_HEADER_LEN; i++)
    {
        result <<= 8;
        result = result | (header[i] & 0x00ff);
    }
    return result;
}
// if failed, return -1
int get_header(FILE *file, int64_t offset, int8_t header[ADTS_HEADER_LEN])
{
    fseek(file, offset, SEEK_SET);
    int readCount = fread(header, sizeof(int8_t), ADTS_HEADER_LEN, file);
    if(readCount < ADTS_HEADER_LEN)
    {
        if(feof(file))
        {
            return EOF;
        }else{
            return -2;
        }
    }
//    uint64_t header_int = convert_header_to_int64(header);
//    LOGD("header = %ld", header_int);
    if(((header[0] & 0xff) == 0xff) && ((header[1] & 0xf0) == 0xf0))
    {
        return 0;
    }
    else
    {
        return -2;
    }
}

int32_t get_sample_rate(int8_t header[ADTS_HEADER_LEN])
{
    int sampleIndex = (header[2] & 0b00111100) >> 2;
    if(sampleIndex >= 13)
    {
        return -1;
    }else{
        return AAC_SUPPORT_SAMPLERATE[sampleIndex];
    }
}

int32_t is_crc(int8_t header[ADTS_HEADER_LEN])
{
    int crcFlag = header[1] & 0b00000001;
    return crcFlag;
}

int32_t get_frame_size(int8_t header[ADTS_HEADER_LEN])
{
    // frame size contains header size and aac data size
    int32_t frameSize = (((header[3] & 0x00000003) << 11) | ((header[4] & 0x000000FF) << 3) | ((header[5] & 0x000000E0) >> 5));
    return frameSize;
}


int64_t get_aac_duration(FILE *file)
{
    fseek(file, 0, SEEK_SET);
    char adif_id[4];
    fread(adif_id, sizeof(char), 4, file);
    if(strcmp(adif_id, "ADIF") == 0)
    {
        return get_adif_aac_duration(file);
    }
    else
    {
        return get_adts_aac_duration(file);
    }
}

int64_t get_adif_aac_duration(FILE *file)
{
    LOGE("get_adif_aac_duration NOT implement");
    return -1;

}

int64_t get_adts_aac_duration(FILE *file)
{

    int8_t header[ADTS_HEADER_LEN];
    if(get_header(file, 0, header) != 0)
    {
        LOGE("get aac header error");
        return -1;
    }
    int sampleRate = get_sample_rate(header);
    if(sampleRate == -1)
    {
        LOGE("get sample rate error");
        return -1;
    }

    LOGD("sample rate = %d", sampleRate);

    int64_t offset = 0;
    int frameCount = 0;
    int frameSize;

    int readHeadResult = 0;
    while(1)
    {
//        LOGD("offset = %ld", offset);
        readHeadResult = get_header(file, offset, header);
        if(readHeadResult == EOF)
        {
            LOGD("reached the end of file");
            break;
        }
        else if(readHeadResult == -2)
        {
            LOGE("read header error");
            return -1;
        }
        else
        {

            frameSize = get_frame_size(header);
//            LOGD("read header success, frame size = %d", frameSize);
            offset += frameSize;
            frameCount++;
        }
    }
    LOGD("total file size is %ld bytes, total %d frames", offset, frameCount);

    double frameDuration = 1024 * 1000.0 / sampleRate;
    int64_t duration = (int64_t)(frameCount * frameDuration);
    LOGD("frame duration = %.2lf, total duration = %ldms", frameDuration, duration);
    return duration;


}