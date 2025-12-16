//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_SPS_H
#define ANALYZE_H264_SPS_SPS_H

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include "VUI.h"

struct SPS {
    uint32_t profile_idc = 0;
    uint32_t constraint_set0_flag = 0;
    uint32_t constraint_set1_flag = 0;
    uint32_t constraint_set2_flag = 0;
    // reserved_zero_5bits, equal to 0
    uint32_t level_idc = 0;
    uint32_t seq_parameter_set_id = 0;
    uint32_t log2_max_frame_num_minus4 = 0;
    uint32_t pic_order_cnt_type = 0;

    // if (pic_order_cnt_type == 0)
    uint32_t log2_max_pic_order_cnt_lsb_minus4 = 0;
    // else if (pic_order_cnt_type == 1)
    uint32_t delta_pic_order_always_zero_flag = 0;
    int32_t offset_for_non_ref_pic = 0;
    int32_t offset_for_top_to_bottom_field = 0;
    uint32_t num_ref_frames_in_pic_order_cnt_cycle = 0;
    // for-loop num_ref_frames_in_pic_order_cnt_cycle
    std::vector<int32_t> offset_for_ref_frame;
    // endif

    uint32_t num_ref_frames = 0;
    uint32_t gaps_in_frame_num_value_allowed_flag = 0;
    uint32_t pic_width_in_mbs_minus1 = 0;
    uint32_t pic_height_in_map_units_minus1 = 0;

    uint32_t frame_mbs_only_flag = 0;
    // if (!frame_mbs_only_flag)
    uint32_t mb_adaptive_frame_field_flag = 0;
    // endif

    uint32_t direct_8x8_inference_flag = 0;

    uint32_t frame_cropping_flag = 0;
    // if (frame_cropping_flag)
    uint32_t frame_crop_left_offset = 0;
    uint32_t frame_crop_right_offset = 0;
    uint32_t frame_crop_top_offset = 0;
    uint32_t frame_crop_bottom_offset = 0;
    // endif

    uint32_t vui_parameters_present_flag = 0;
    // if (vui_parameters_present_flag)
    VUI *vui = nullptr;
    // endif

    // rbsp_trailing_bits()

    ~SPS() {
        if (vui != nullptr) {
            delete vui;
        }
    }
};

#endif //ANALYZE_H264_SPS_SPS_H