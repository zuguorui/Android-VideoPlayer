//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_NAL_H
#define ANALYZE_H264_SPS_NAL_H
#include <cstdint>
#include <vector>

struct NAL {
    int32_t forbidden_zero_bit;
    uint32_t nal_ref_idc;
    uint32_t nal_unit_type;
    std::vector<uint8_t> rbsp;
};

#endif //ANALYZE_H264_SPS_NAL_H