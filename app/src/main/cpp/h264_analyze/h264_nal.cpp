//
// Created by WangXi on 2025/11/6.
//

#include "h264_nal.h"

#include "Log.h"

#define TAG "H264_NAL"

using namespace std;

NAL* nal_unit(BufferBitReader &reader);
VUI* vui_parameters(BufferBitReader &reader);
HRD* hrd_parameters(BufferBitReader &reader);

NAL* nal_unit(BufferBitReader &reader) {
    NAL *nal = new NAL();
    nal->forbidden_zero_bit = reader.f(1);
    nal->nal_ref_idc = reader.u(2);
    nal->nal_unit_type = reader.u(5);
    vector<uint8_t> pureNAL;
    while (reader.moreByteInByteStream()) {
        if (reader.hasRemainBytes(2) && reader.nextBits(24) == 0x000003) {
            pureNAL.push_back(reader.b(8));
            pureNAL.push_back(reader.b(8));
            reader.f(8);
        } else {
            pureNAL.push_back(reader.b(8));
        }
    }
    nal->rbsp = pureNAL;
    return nal;
}

NAL* parse_nal(std::vector<uint8_t> &nalBody) {
    BufferBitReader reader(nalBody);
    return nal_unit(reader);
}

VUI* vui_parameters(BufferBitReader &reader) {
    VUI *vui = new VUI();
    vui->aspect_ratio_info_present_flag = reader.u(1);
    if (vui->aspect_ratio_info_present_flag) {
        vui->aspect_ratio_idc = reader.u(8);
        if (vui->aspect_ratio_idc == 255) {
            vui->sar_width = reader.u(16);
            vui->sar_height = reader.u(16);
        }
    }
    vui->overscan_info_present_flag = reader.u(1);
    if (vui->overscan_info_present_flag) {
        vui->overscan_appropriate_flag = reader.u(1);
    }
    vui->video_signal_type_present_flag = reader.u(1);
    if (vui->video_signal_type_present_flag) {
        vui->video_format = reader.u(3);
        vui->video_full_range_flag = reader.u(1);
        vui->colour_description_present_flag = reader.u(1);
        if (vui->colour_description_present_flag) {
            vui->colour_primaries = reader.u(8);
            vui->transfer_characteristics = reader.u(8);
            vui->matrix_coefficients = reader.u(8);
        }
    }
    vui->chroma_loc_info_present_flag = reader.u(1);
    if (vui->chroma_loc_info_present_flag) {
        vui->chroma_sample_loc_type_top_field = reader.ue();
        vui->chroma_sample_loc_type_bottom_field = reader.ue();
    }
    //LOGD(TAG, "timing_info_present_flag is byte aligned: %d", reader.byteAligned());
    vui->timing_info_present_flag = reader.u(1);
    if (vui->timing_info_present_flag) {
        //LOGD(TAG, "num_units_in_tick is byte aligned: %d", reader.byteAligned());
        vui->num_units_in_tick = reader.u(32);
        vui->time_scale = reader.u(32);
        vui->fixed_frame_rate_flag = reader.u(1);
    }

    vui->nal_hrd_parameters_present_flag = reader.u(1);
    if (vui->nal_hrd_parameters_present_flag) {
        vui->nal_hrd = hrd_parameters(reader);
    }
    vui->vcl_hrd_parameters_present_flag = reader.u(1);
    if (vui->vcl_hrd_parameters_present_flag) {
        vui->vcl_hrd = hrd_parameters(reader);
    }
    if (vui->nal_hrd_parameters_present_flag || vui->vcl_hrd_parameters_present_flag) {
        vui->low_delay_hrd_flag = reader.u(1);
    }
    vui->pic_struct_present_flag = reader.u(1);
    vui->bitstream_restriction_flag = reader.u(1);
    if (vui->bitstream_restriction_flag) {
        vui->motion_vectors_over_pic_boundaries_flag = reader.u(1);
        vui->max_bytes_per_pic_denom = reader.ue();
        vui->max_bits_per_mb_denom = reader.ue();
        vui->log2_max_mv_length_horizontal = reader.ue();
        vui->log2_max_mv_length_vertical = reader.ue();
        vui->num_reorder_frames = reader.ue();
        vui->max_dec_frame_buffering = reader.ue();
    }
    return vui;
}

HRD* hrd_parameters(BufferBitReader &reader) {
    HRD *hrd = new HRD();
    hrd->cpb_cnt_minus1 = reader.ue();
    hrd->bitrate_scale = reader.u(4);
    hrd->cpb_size_scale = reader.u(4);
    for (int i = 0; i < hrd->cpb_cnt_minus1; i++) {
        hrd->bit_rate_value_minus1.push_back(reader.ue());
        hrd->cpb_size_value_minus1.push_back(reader.ue());
        hrd->cbr_flag.push_back(reader.u(1));
    }
    hrd->initial_cpb_removal_delay_length_minus1 = reader.u(5);
    hrd->cpb_removal_delay_length_minus1 = reader.u(5);
    hrd->dpb_output_delay_length_minus1 = reader.u(5);
    hrd->time_offset_length = reader.u(5);
    return hrd;
}

std::vector<uint8_t> parse_nal_body(BitReader &reader) {
    vector<uint8_t> nalData;
    uint32_t buf = 0;

    while (reader.nextBits(24) != 0x000001 && reader.nextBits(32) != 0x00000001) {
        reader.readByte();
        if (!reader.moreByteInByteStream()) {
            return nalData;
        }
    }

    if (reader.nextBits(24) != 0x000001) {
        reader.readByte();
    }

    reader.readBits(24);

    if (!reader.moreByteInByteStream()) {
        return nalData;
    }

    while (reader.moreByteInByteStream() && reader.nextBits(24) != 0x000001 && reader.nextBits(32) != 0x00000001) {
        nalData.push_back(reader.readByte());
    }

    // while (nalData[nalData.size() - 1] == 0) {
    //     nalData.pop_back();
    // }

    LOGD(TAG, "NAL size: %lu", nalData.size());
    //LOGD(TAG, "current bit position: %lld", reader.currentBitPosition());
    return nalData;
}

int get_nal_type(std::vector<uint8_t> &nalBody) {
    if (nalBody.empty()) {
        return -1;
    }
    uint8_t nalHead = nalBody[0];
    int nalType = nalHead & 0x1F;
    return nalType;
}

NAL* parse_nal(BitReader &reader) {
    vector<uint8_t> nalData = parse_nal_body(reader);
    if (nalData.empty()) {
        return nullptr;
    }
    BufferBitReader bufferBitReader(nalData);
    return nal_unit(bufferBitReader);
}

SPS* parse_sps(NAL *nal) {
    if (nal->nal_unit_type != 7) {
        return nullptr;
    }
    BufferBitReader reader(nal->rbsp);
    SPS *sps = new SPS();
    sps->profile_idc = reader.u(8);
    sps->constraint_set0_flag = reader.u(1);
    sps->constraint_set1_flag = reader.u(1);
    sps->constraint_set2_flag = reader.u(1);
    reader.u(5);
    sps->level_idc = reader.u(8);
    sps->seq_parameter_set_id = reader.ue();
    sps->log2_max_frame_num_minus4 = reader.ue();
    sps->pic_order_cnt_type = reader.ue();
    if (sps->pic_order_cnt_type == 0) {
        sps->log2_max_pic_order_cnt_lsb_minus4 = reader.ue();
    } else if (sps->pic_order_cnt_type == 1) {
        sps->delta_pic_order_always_zero_flag = reader.u(1);
        sps->offset_for_non_ref_pic = reader.se();
        sps->offset_for_top_to_bottom_field = reader.se();
        sps->num_ref_frames_in_pic_order_cnt_cycle = reader.ue();
        for (int i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
            sps->offset_for_ref_frame.push_back(reader.se());
        }
    }
    sps->num_ref_frames = reader.ue();
    sps->gaps_in_frame_num_value_allowed_flag = reader.u(1);
    sps->pic_width_in_mbs_minus1 = reader.ue();
    sps->pic_height_in_map_units_minus1 = reader.ue();
    sps->frame_mbs_only_flag = reader.u(1);
    if (!sps->frame_mbs_only_flag) {
        sps->mb_adaptive_frame_field_flag = reader.u(1);
    }
    sps->direct_8x8_inference_flag = reader.u(1);
    sps->frame_cropping_flag = reader.u(1);
    if (sps->frame_cropping_flag) {
        sps->frame_crop_left_offset = reader.ue();
        sps->frame_crop_right_offset = reader.ue();
        sps->frame_crop_top_offset = reader.ue();
        sps->frame_crop_bottom_offset = reader.ue();
    }
    sps->vui_parameters_present_flag = reader.u(1);
    if (sps->vui_parameters_present_flag) {
        sps->vui = vui_parameters(reader);
    }
    reader.alignToNextByte();
    return sps;
}

PPS* parse_pps(NAL *nal) {
    if (nal->nal_unit_type != 8) {
        return nullptr;
    }
    BufferBitReader reader(nal->rbsp);
    PPS *pps = new PPS();
    pps->pic_parameter_set_id = reader.ue();
    pps->seq_parameter_set_id = reader.ue();
    pps->entropy_coding_mode_flag = reader.u(1);
    pps->pic_order_present_flag = reader.u(1);
    pps->num_slice_groups_minus1 = reader.ue();
    if (pps->num_slice_groups_minus1 > 0) {
        pps->slice_group_map_type = reader.ue();
        if (pps->slice_group_map_type == 0) {
            for (int i = 0; i <= pps->num_slice_groups_minus1; i++) {
                pps->run_length_minus1.push_back(reader.ue());
            }
        } else if (pps->slice_group_map_type == 2) {
            for (int i = 0; i < pps->num_slice_groups_minus1; i++) {
                pps->top_left.push_back(reader.ue());
                pps->bottom_right.push_back(reader.ue());
            }
        } else if (pps->slice_group_map_type == 3 || pps->slice_group_map_type == 4 || pps->slice_group_map_type == 5) {
            pps->slice_group_change_direction_flag = reader.u(1);
            pps->slice_group_change_rate_minus1 = reader.ue();
        } else if (pps->slice_group_map_type == 6) {
            pps->pic_size_in_map_units_minus1 = reader.ue();
            for (int i = 0; i <= pps->pic_size_in_map_units_minus1; i++) {
                pps->slice_group_id.push_back(reader.ue());
            }
        }
    }
    pps->num_ref_idx_l0_active_minus1 = reader.ue();
    pps->num_ref_idx_l1_active_minus1 = reader.ue();
    pps->weighted_pred_flag = reader.u(1);
    pps->weighted_bipred_idc = reader.u(2);
    pps->pic_init_qp_minus26 = reader.se();
    pps->pic_init_qs_minus26 = reader.se();
    pps->chroma_qp_index_offset = reader.se();
    pps->deblocking_filter_control_present_flag = reader.u(1);
    pps->constrained_intra_pred_flag = reader.u(1);
    pps->redundant_pic_cnt_present_flag = reader.u(1);
    reader.alignToNextByte();
    return pps;
}