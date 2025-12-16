//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_VUI_H
#define ANALYZE_H264_SPS_VUI_H

#include "HRD.h"

struct VUI {
    uint32_t aspect_ratio_info_present_flag = 0;
    // if #1 (aspect_ratio_info_present_flag)
    uint32_t aspect_ratio_idc = 0;
    // if #1.1 (aspect_ration_idc == Extended_SAR) Extended_SAR = 255
    uint32_t sar_width = 0;
    uint32_t sar_height = 0;
    // endif #1.1
    // endif #1
    uint32_t overscan_info_present_flag = 0;
    // if (overscan_info_present_flag)
    uint32_t overscan_appropriate_flag = 0;
    // endif
    uint32_t video_signal_type_present_flag = 0;
    // if #1 (video_signal_type_present_flag)
    uint32_t video_format = 0;
    uint32_t video_full_range_flag = 0;
    uint32_t colour_description_present_flag = 0;
    // if #1.1 (colour_description_present_flag)
    uint32_t colour_primaries = 0;
    uint32_t transfer_characteristics = 0;
    uint32_t matrix_coefficients = 0;
    // endif #1.1
    // endif #1
    uint32_t chroma_loc_info_present_flag = 0;
    // if (chroma_loc_info_present_flag)
    uint32_t chroma_sample_loc_type_top_field = 0;
    uint32_t chroma_sample_loc_type_bottom_field = 0;
    // endif
    uint32_t timing_info_present_flag = 0;
    // if (timing_info_present_flag)
    uint32_t num_units_in_tick = 0;
    uint32_t time_scale = 0;
    uint32_t fixed_frame_rate_flag = 0;
    // endif
    uint32_t nal_hrd_parameters_present_flag = 0;
    // if (nal_hrd_parameters_present_flag)
    HRD *nal_hrd = nullptr;
    // endif

    uint32_t vcl_hrd_parameters_present_flag = 0;
    // if (vcl_hrd_parameters_present_flag)
    HRD *vcl_hrd = nullptr;
    // endif

    // if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag)
    uint32_t low_delay_hrd_flag = 0;
    // endif

    uint32_t pic_struct_present_flag = 0;
    uint32_t bitstream_restriction_flag = 0;
    // if (bitstream_restriction_flag)
    uint32_t motion_vectors_over_pic_boundaries_flag = 0;
    uint32_t max_bytes_per_pic_denom = 0;
    uint32_t max_bits_per_mb_denom = 0;
    uint32_t log2_max_mv_length_horizontal = 0;
    uint32_t log2_max_mv_length_vertical = 0;
    uint32_t num_reorder_frames = 0;
    uint32_t max_dec_frame_buffering = 0;
    // endif

    ~VUI() {
        if (nal_hrd != nullptr) {
            delete nal_hrd;
        }
        if (vcl_hrd != nullptr) {
            delete vcl_hrd;
        }
    }

};

#endif //ANALYZE_H264_SPS_VUI_H