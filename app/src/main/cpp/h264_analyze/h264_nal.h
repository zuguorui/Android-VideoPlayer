//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_H264_NAL_H
#define ANALYZE_H264_SPS_H264_NAL_H

#include "FileBitReader.h"
#include "BufferBitReader.h"
#include "NAL.h"
#include "SPS.h"
#include "PPS.h"
#include <vector>

/**
 * 从流中解析出一个NAL单元。
 * @param reader 文件比特流读取器。解析是从reader的当前位置开始。
 * @return NAL，调用者需自己释放内存
 */
NAL* parse_nal(BitReader &reader);
NAL* parse_nal(std::vector<uint8_t> &nalBody);
SPS* parse_sps(NAL *nal);
PPS* parse_pps(NAL *nal);
std::vector<uint8_t> parse_nal_body(BitReader &reader);
int get_nal_type(std::vector<uint8_t> &nalBody);

#endif //ANALYZE_H264_SPS_H264_NAL_H