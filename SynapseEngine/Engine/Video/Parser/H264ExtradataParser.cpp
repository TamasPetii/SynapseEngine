// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "H264ExtradataParser.h"
#include <cstring>

namespace Syn
{
    namespace
    {
        struct H264BitReader {
            const uint8_t* data;
            size_t size;
            size_t bitPos = 0;

            uint32_t ReadBits(int n) {
                uint32_t val = 0;
                for (int i = 0; i < n; ++i) {
                    if (bitPos / 8 >= size) return val;
                    int bytePos = bitPos / 8;
                    int bitOffset = 7 - (bitPos % 8);
                    val = (val << 1) | ((data[bytePos] >> bitOffset) & 1);
                    bitPos++;
                }
                return val;
            }

            uint32_t ReadUE() {
                int leadingZeroBits = -1;
                for (int b = 0; !b; leadingZeroBits++) {
                    b = ReadBits(1);
                }
                return (1 << leadingZeroBits) - 1 + ReadBits(leadingZeroBits);
            }

            int32_t ReadSE() {
                uint32_t ue = ReadUE();
                int32_t se = (ue + 1) / 2;
                return (ue % 2 == 0) ? -se : se;
            }
        };

        std::vector<uint8_t> RemoveEmulationPrevention(const std::vector<uint8_t>& nalu) {
            std::vector<uint8_t> clean;
            clean.reserve(nalu.size());
            for (size_t i = 0; i < nalu.size(); ++i) {
                if (i >= 2 && nalu[i] == 0x03 && nalu[i - 1] == 0x00 && nalu[i - 2] == 0x00) {
                    continue;
                }
                clean.push_back(nalu[i]);
            }
            return clean;
        }
    }

    bool H264ExtradataParser::Parse(const std::vector<uint8_t>& extradata, StdVideoH264SequenceParameterSet& outSps, StdVideoH264PictureParameterSet& outPps)
    {
        std::memset(&outSps, 0, sizeof(StdVideoH264SequenceParameterSet));
        std::memset(&outPps, 0, sizeof(StdVideoH264PictureParameterSet));

        if (extradata.size() < 7 || extradata[0] != 1) return false;

        int numSps = extradata[5] & 0x1F;
        size_t offset = 6;
        std::vector<uint8_t> spsNalu;

        for (int i = 0; i < numSps; ++i) {
            uint16_t length = (extradata[offset] << 8) | extradata[offset + 1];
            offset += 2;
            if (i == 0) spsNalu.assign(extradata.begin() + offset, extradata.begin() + offset + length);
            offset += length;
        }

        int numPps = extradata[offset++];
        std::vector<uint8_t> ppsNalu;
        for (int i = 0; i < numPps; ++i) {
            uint16_t length = (extradata[offset] << 8) | extradata[offset + 1];
            offset += 2;
            if (i == 0) ppsNalu.assign(extradata.begin() + offset, extradata.begin() + offset + length);
            offset += length;
        }

        if (!spsNalu.empty()) {
            auto cleanSps = RemoveEmulationPrevention(spsNalu);
            H264BitReader br{ cleanSps.data(), cleanSps.size() };
            br.ReadBits(8);

            outSps.profile_idc = (StdVideoH264ProfileIdc)br.ReadBits(8);
            outSps.flags.constraint_set0_flag = br.ReadBits(1);
            outSps.flags.constraint_set1_flag = br.ReadBits(1);
            outSps.flags.constraint_set2_flag = br.ReadBits(1);
            outSps.flags.constraint_set3_flag = br.ReadBits(1);
            outSps.flags.constraint_set4_flag = br.ReadBits(1);
            outSps.flags.constraint_set5_flag = br.ReadBits(1);
            br.ReadBits(2);

            outSps.level_idc = (StdVideoH264LevelIdc)br.ReadBits(8);
            outSps.seq_parameter_set_id = static_cast<uint8_t>(br.ReadUE());

            if (outSps.profile_idc == 100 || outSps.profile_idc == 110 || outSps.profile_idc == 122 || outSps.profile_idc == 244 || outSps.profile_idc == 44 || outSps.profile_idc == 83 || outSps.profile_idc == 86 || outSps.profile_idc == 118 || outSps.profile_idc == 128) {
                uint32_t chroma = br.ReadUE();
                if (chroma == 3) br.ReadBits(1);
                br.ReadUE();
                br.ReadUE();
                br.ReadBits(1);
                if (br.ReadBits(1)) {}
            }

            outSps.log2_max_frame_num_minus4 = static_cast<uint8_t>(br.ReadUE());
            outSps.pic_order_cnt_type = static_cast<StdVideoH264PocType>(br.ReadUE());

            if (outSps.pic_order_cnt_type == STD_VIDEO_H264_POC_TYPE_0) {
                outSps.log2_max_pic_order_cnt_lsb_minus4 = static_cast<uint8_t>(br.ReadUE());
            }
            else if (outSps.pic_order_cnt_type == STD_VIDEO_H264_POC_TYPE_1) {
                outSps.flags.delta_pic_order_always_zero_flag = br.ReadBits(1);
                outSps.offset_for_non_ref_pic = br.ReadSE();
                outSps.offset_for_top_to_bottom_field = br.ReadSE();
                uint32_t num_ref = br.ReadUE();
                for (uint32_t i = 0; i < num_ref; ++i) br.ReadSE();
            }

            outSps.max_num_ref_frames = static_cast<uint8_t>(br.ReadUE());
            outSps.flags.gaps_in_frame_num_value_allowed_flag = br.ReadBits(1);
            outSps.pic_width_in_mbs_minus1 = br.ReadUE();
            outSps.pic_height_in_map_units_minus1 = br.ReadUE();

            outSps.flags.frame_mbs_only_flag = br.ReadBits(1);
            if (!outSps.flags.frame_mbs_only_flag) {
                outSps.flags.mb_adaptive_frame_field_flag = br.ReadBits(1);
            }
            outSps.flags.direct_8x8_inference_flag = br.ReadBits(1);
            outSps.flags.frame_cropping_flag = br.ReadBits(1);

            if (outSps.flags.frame_cropping_flag) {
                outSps.frame_crop_left_offset = br.ReadUE();
                outSps.frame_crop_right_offset = br.ReadUE();
                outSps.frame_crop_top_offset = br.ReadUE();
                outSps.frame_crop_bottom_offset = br.ReadUE();
            }
        }

        if (!ppsNalu.empty()) {
            auto cleanPps = RemoveEmulationPrevention(ppsNalu);
            H264BitReader br{ cleanPps.data(), cleanPps.size() };
            br.ReadBits(8);

            outPps.pic_parameter_set_id = static_cast<uint8_t>(br.ReadUE());
            outPps.seq_parameter_set_id = static_cast<uint8_t>(br.ReadUE());
            outPps.flags.entropy_coding_mode_flag = br.ReadBits(1);
            outPps.flags.bottom_field_pic_order_in_frame_present_flag = br.ReadBits(1);

            uint32_t num_slice_groups_minus1 = br.ReadUE();
            if (num_slice_groups_minus1 > 0) {
                br.ReadUE();
            }

            outPps.num_ref_idx_l0_default_active_minus1 = static_cast<uint8_t>(br.ReadUE());
            outPps.num_ref_idx_l1_default_active_minus1 = static_cast<uint8_t>(br.ReadUE());
            outPps.flags.weighted_pred_flag = br.ReadBits(1);
            outPps.weighted_bipred_idc = static_cast<StdVideoH264WeightedBipredIdc>(br.ReadBits(2));
            outPps.pic_init_qp_minus26 = static_cast<int8_t>(br.ReadSE());
            outPps.pic_init_qs_minus26 = static_cast<int8_t>(br.ReadSE());
            outPps.chroma_qp_index_offset = static_cast<int8_t>(br.ReadSE());
            outPps.flags.deblocking_filter_control_present_flag = br.ReadBits(1);
            outPps.flags.constrained_intra_pred_flag = br.ReadBits(1);
            outPps.flags.redundant_pic_cnt_present_flag = br.ReadBits(1);
        }
        return true;
    }
}