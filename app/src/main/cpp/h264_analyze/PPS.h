//
// Created by zu on 2025/12/10.
//

#ifndef LB_CAMERA_SDK_V2_PPS_H
#define LB_CAMERA_SDK_V2_PPS_H

#include <stdlib.h>
#include <vector>

struct PPS {
    uint32_t pic_parameter_set_id = 0;
    uint32_t seq_parameter_set_id = 0;
    uint32_t entropy_coding_mode_flag = 0;
    uint32_t pic_order_present_flag = 0;
    uint32_t num_slice_groups_minus1 = 0;
    // if (num_slice_groups_minus1 > 0)
    uint32_t slice_group_map_type = 0;
    // if (slice_group_map_type == 0)
    // for(0 <= i <= num_slice_groups_minus1)
    std::vector<uint32_t> run_length_minus1;
    // end for
    // else if (slice_group_map_type == 2)
    // for (0 <= i < num_slice_groups_minus1)
    std::vector<uint32_t> top_left;
    std::vector<uint32_t> bottom_right;
    // end for
    // else if (slice_group_map_type == 3 | 4 | 5)
    uint32_t slice_group_change_direction_flag = 0;
    uint32_t slice_group_change_rate_minus1 = 0;
    // else if (slice_group_map_type == 6) {
    uint32_t pic_size_in_map_units_minus1 = 0;
    // for (0 <= i <= pic_size_in_map_units_minus1)
    std::vector<uint32_t> slice_group_id;
    // end for
    // endif

    uint32_t num_ref_idx_l0_active_minus1 = 0;
    uint32_t num_ref_idx_l1_active_minus1 = 0;
    uint32_t weighted_pred_flag = 0;
    uint32_t weighted_bipred_idc = 0;
    int32_t pic_init_qp_minus26 = 0; // relative to 26
    int32_t pic_init_qs_minus26 = 0; // relative to 26
    int32_t chroma_qp_index_offset = 0;
    uint32_t deblocking_filter_control_present_flag = 0;
    uint32_t constrained_intra_pred_flag = 0;
    uint32_t redundant_pic_cnt_present_flag = 0;

    // rbsp_trailing_bits()
};

#endif //LB_CAMERA_SDK_V2_PPS_H
