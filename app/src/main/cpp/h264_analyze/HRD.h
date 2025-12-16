//
// Created by WangXi on 2025/11/7.
//

#ifndef ANALYZE_H264_SPS_HRD_H
#define ANALYZE_H264_SPS_HRD_H

#include <vector>

struct HRD {
    uint32_t cpb_cnt_minus1 = 0;
    uint32_t bitrate_scale = 0;
    uint32_t cpb_size_scale = 0;
    // for (SchedSelIdx = 0; SchedSelIdx <= cpb_cnt_minus1; SchedSelIdx++)
    std::vector<uint32_t> bit_rate_value_minus1;
    std::vector<uint32_t> cpb_size_value_minus1;
    std::vector<uint32_t> cbr_flag;
    // end for
    uint32_t initial_cpb_removal_delay_length_minus1 = 0;
    uint32_t cpb_removal_delay_length_minus1 = 0;
    uint32_t dpb_output_delay_length_minus1 = 0;
    uint32_t time_offset_length = 0;
};

#endif //ANALYZE_H264_SPS_HRD_H