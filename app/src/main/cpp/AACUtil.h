//
// Created by incus-pc-2 on 2020/1/7.
//

#ifndef FFMPEGAUDIOPLAYER_AACUTIL_H
#define FFMPEGAUDIOPLAYER_AACUTIL_H

#include <stdlib.h>
#define ADTS_FIXED_HEADER_BIT_SIZE 28
#define ADTS_VARIABLE_HEADER_BIT_SIZE 28
#define ADTS_HEADER_BIT_SIZE 56
#define ADTS_HEADER_LEN 7
#define NO_CRC_HEADER_SIZE 9
#define CRC_HEADER_SIZE ADTS_HEADER_LEN



int64_t get_aac_duration(FILE *file);

int64_t get_adif_aac_duration(FILE *file);

int64_t get_adts_aac_duration(FILE *file);


const static int32_t AAC_SUPPORT_SAMPLERATE[13] = {96000, 88200, 64000, 48000, 44100, 32000,
                                               24000, 22050, 16000, 12000, 11025, 8000,
                                               7350};

#endif //FFMPEGAUDIOPLAYER_AACUTIL_H
