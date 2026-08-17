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

#include "VulkanGpuVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include <algorithm>

namespace Syn
{
    namespace {
        struct BitReader {
            const uint8_t* data;
            size_t size;
            size_t bitPos = 0;

            uint32_t ReadBits(int n) {
                uint32_t val = 0;
                for (int i = 0; i < n; ++i) {
                    if (bitPos / 8 >= size) return val;
                    int bytePos = static_cast<int>(bitPos / 8);
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
        };

        std::vector<uint8_t> RemoveEmulationPrevention(const uint8_t* data, size_t size) {
            std::vector<uint8_t> clean;
            clean.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                if (i >= 2 && data[i] == 0x03 && data[i - 1] == 0x00 && data[i - 2] == 0x00) {
                    continue;
                }
                clean.push_back(data[i]);
            }
            return clean;
        }

        struct SliceHeaderInfo {
            bool isIdr = false;
            bool isIntra = false;
            bool isReference = false;
            uint16_t frameNum = 0;
            uint16_t picOrderCntLsb = 0;
            bool isValid = false;
        };

        SliceHeaderInfo ParseAdvancedSliceHeader(const std::vector<uint8_t>& bitstream, const StdVideoH264SequenceParameterSet& sps) {
            SliceHeaderInfo info;

            for (size_t i = 0; i + 3 < bitstream.size(); ) {
                uint32_t startCodeSize = 0;

                if (bitstream[i] == 0x00 && bitstream[i + 1] == 0x00 && bitstream[i + 2] == 0x01) {
                    startCodeSize = 3;
                }
                else if (i + 4 <= bitstream.size() &&
                    bitstream[i] == 0x00 && bitstream[i + 1] == 0x00 &&
                    bitstream[i + 2] == 0x00 && bitstream[i + 3] == 0x01) {
                    startCodeSize = 4;
                }

                if (startCodeSize > 0) {
                    uint8_t nalType = bitstream[i + startCodeSize] & 0x1F;
                    uint8_t nalRefIdc = (bitstream[i + startCodeSize] >> 5) & 0x03;

                    if (nalType >= 1 && nalType <= 5) {
                        info.isIdr = (nalType == 5);
                        info.isIntra = (nalType == 5);
                        info.isReference = (nalRefIdc > 0);

                        size_t naluStart = i + startCodeSize;
                        size_t naluEnd = bitstream.size();

                        for (size_t j = naluStart; j + 3 < bitstream.size() && j - naluStart < 64; ++j) {
                            if ((bitstream[j] == 0x00 && bitstream[j + 1] == 0x00 && bitstream[j + 2] == 0x01) ||
                                (j + 4 <= bitstream.size() && bitstream[j] == 0x00 && bitstream[j + 1] == 0x00 && bitstream[j + 2] == 0x00 && bitstream[j + 3] == 0x01)) {
                                naluEnd = j;
                                break;
                            }
                        }

                        size_t parseSize = std::min(naluEnd - naluStart - 1, static_cast<size_t>(64));
                        std::vector<uint8_t> cleanNalu = RemoveEmulationPrevention(bitstream.data() + naluStart + 1, parseSize);

                        BitReader br{ cleanNalu.data(), cleanNalu.size() };

                        br.ReadUE();
                        uint32_t sliceType = br.ReadUE();

                        if (sliceType == 2 || sliceType == 4 || sliceType == 7 || sliceType == 9) {
                            info.isIntra = true;
                        }

                        br.ReadUE();

                        uint32_t log2_max_frame_num = sps.log2_max_frame_num_minus4 + 4;
                        info.frameNum = br.ReadBits(log2_max_frame_num);

                        if (!sps.flags.frame_mbs_only_flag) {
                            bool field_pic_flag = br.ReadBits(1);
                            if (field_pic_flag) {
                                br.ReadBits(1);
                            }
                        }

                        if (info.isIdr) {
                            br.ReadUE();
                        }

                        if (sps.pic_order_cnt_type == STD_VIDEO_H264_POC_TYPE_0) {
                            uint32_t log2_max_poc = sps.log2_max_pic_order_cnt_lsb_minus4 + 4;
                            info.picOrderCntLsb = br.ReadBits(log2_max_poc);
                        }

                        info.isValid = true;
                        break;
                    }
                    i += startCodeSize;
                }
                else {
                    i++;
                }
            }
            return info;
        }
    }

    VulkanGpuVideoUploader::VulkanGpuVideoUploader(
        uint32_t width,
        uint32_t height,
        uint32_t bufferCount,
        const std::vector<uint8_t>& extradata,
        std::shared_ptr<IH264ExtradataParser> parser)
        : _width(width), _height(height), _extradata(extradata), _parser(parser)
    {
        _textures.resize(bufferCount);
    }

    VulkanGpuVideoUploader::~VulkanGpuVideoUploader()
    {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();

        if (_ycbcrConversion != VK_NULL_HANDLE) {
            vkDestroySamplerYcbcrConversion(device, _ycbcrConversion, nullptr);
            _ycbcrConversion = VK_NULL_HANDLE;
        }

        if (_sessionParams != VK_NULL_HANDLE) {
            vkDestroyVideoSessionParametersKHR(device, _sessionParams, nullptr);
            _sessionParams = VK_NULL_HANDLE;
        }

        if (_videoSession != VK_NULL_HANDLE) {
            vkDestroyVideoSessionKHR(device, _videoSession, nullptr);
            _videoSession = VK_NULL_HANDLE;
        }

        for (auto mem : _sessionMemories) {
            if (mem != VK_NULL_HANDLE) {
                vkFreeMemory(device, mem, nullptr);
            }
        }
        _sessionMemories.clear();
    }

    void VulkanGpuVideoUploader::ParseSliceHeader(const std::vector<uint8_t>& bitstream, bool& outIsIdr, bool& outIsIntra, bool& outIsReference)
    {
        outIsIdr = false;
        outIsIntra = false;
        outIsReference = false;

        for (size_t i = 0; i + 3 < bitstream.size(); ) {
            uint32_t startCodeSize = 0;

            if (bitstream[i] == 0x00 && bitstream[i + 1] == 0x00 && bitstream[i + 2] == 0x01) {
                startCodeSize = 3;
            }
            else if (i + 4 <= bitstream.size() &&
                bitstream[i] == 0x00 && bitstream[i + 1] == 0x00 &&
                bitstream[i + 2] == 0x00 && bitstream[i + 3] == 0x01) {
                startCodeSize = 4;
            }

            if (startCodeSize > 0) {
                uint8_t nalType = bitstream[i + startCodeSize] & 0x1F;
                uint8_t nalRefIdc = (bitstream[i + startCodeSize] >> 5) & 0x03;

                if (nalType == 5) {
                    outIsIdr = true;
                    outIsIntra = true;
                    outIsReference = true;
                    return;
                }
                else if (nalType >= 1 && nalType <= 4) {
                    outIsReference = (nalRefIdc > 0);
                    return;
                }
                i += startCodeSize;
            }
            else {
                i++;
            }
        }
    }

    VideoUploadResult VulkanGpuVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader)
    {
        VideoUploadResult result;
        result.isFrameReady = false;

        if (data.bitstreamData.empty()) {
            return result;
        }

        auto context = ServiceLocator::Get<Vk::Context>();
        auto deviceObj = context->GetDevice();
        auto device = deviceObj->Handle();
        VkPhysicalDevice physicalDevice = context->GetPhysicalDevice()->Handle();

        StdVideoH264SequenceParameterSet sps{};
        StdVideoH264PictureParameterSet pps{};
        bool hasParams = _parser && _parser->Parse(_extradata, sps, pps);

        if (!hasParams) {
            return result;
        }

        uint32_t codedWidth = (sps.pic_width_in_mbs_minus1 + 1) * 16;
        uint32_t codedHeight = (2 - sps.flags.frame_mbs_only_flag) * (sps.pic_height_in_map_units_minus1 + 1) * 16;

        VkVideoDecodeH264ProfileInfoKHR h264ProfileInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR };
        h264ProfileInfo.stdProfileIdc = static_cast<StdVideoH264ProfileIdc>(sps.profile_idc);
        h264ProfileInfo.pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_PROGRESSIVE_KHR;

        VkVideoProfileInfoKHR videoProfileInfo{ VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR };
        videoProfileInfo.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR;
        videoProfileInfo.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
        videoProfileInfo.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
        videoProfileInfo.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
        videoProfileInfo.pNext = &h264ProfileInfo;

        VkVideoProfileListInfoKHR videoProfileList{ VK_STRUCTURE_TYPE_VIDEO_PROFILE_LIST_INFO_KHR };
        videoProfileList.profileCount = 1;
        videoProfileList.pProfiles = &videoProfileInfo;

        size_t rawSize = data.bitstreamData.size();
        size_t alignment = 256;
        size_t byteSize = (rawSize + alignment - 1) & ~(alignment - 1);

        std::vector<uint8_t> alignedData = data.bitstreamData;
        alignedData.resize(byteSize, 0);

        result.bitstreamBuffer = Vk::BufferFactory::CreateVideoBitstream(byteSize, &videoProfileList);
        result.bitstreamBuffer->Write(alignedData.data(), byteSize, 0);

        if (!_textures[0]) {
            _spsId = sps.seq_parameter_set_id;
            _ppsId = pps.pic_parameter_set_id;

            VkVideoDecodeH264CapabilitiesKHR h264Capabilities{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR };
            VkVideoDecodeCapabilitiesKHR decodeCapabilities{ VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR };
            decodeCapabilities.pNext = &h264Capabilities;
            VkVideoCapabilitiesKHR videoCapabilities{ VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR };
            videoCapabilities.pNext = &decodeCapabilities;

            vkGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, &videoProfileInfo, &videoCapabilities);

            _maxDpbSlots = std::min(videoCapabilities.maxDpbSlots, 16u);
            _textures.resize(_maxDpbSlots + 8);

            Vk::ImageConfig imgConfig{};
            imgConfig.width = _width;
            imgConfig.height = _height;
            imgConfig.depth = 1;
            imgConfig.format = data.format;
            imgConfig.mipLevels = 1;
            imgConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imgConfig.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            imgConfig.videoProfileList = &videoProfileList;

            imgConfig.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_R8_UNORM,
                .aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT
                });

            imgConfig.AddView(Vk::ImageViewNames::Luma, Vk::ImageViewConfig{
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_R8_UNORM,
                .aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT
                });

            imgConfig.AddView(Vk::ImageViewNames::Chroma, Vk::ImageViewConfig{
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = VK_FORMAT_R8G8_UNORM,
                .aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT
                });

            for (auto& tex : _textures) {
                tex = std::make_shared<Vk::Image>(imgConfig);
            }

            Vk::ImageConfig dpbConfig{};
            dpbConfig.width = codedWidth;
            dpbConfig.height = codedHeight;
            dpbConfig.depth = 1;
            dpbConfig.format = data.format;
            dpbConfig.mipLevels = 1;
            dpbConfig.usage = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
            dpbConfig.videoProfileList = &videoProfileList;

            _dpbTextures.resize(_maxDpbSlots);
            _dpbResources.resize(_maxDpbSlots);
            _dpbSlots.resize(_maxDpbSlots);

            for (uint32_t i = 0; i < _maxDpbSlots; ++i) {
                _dpbTextures[i] = std::make_shared<Vk::Image>(dpbConfig);

                _dpbResources[i].sType = VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR;
                _dpbResources[i].pNext = nullptr;
                _dpbResources[i].imageViewBinding = _dpbTextures[i]->GetView();
                _dpbResources[i].codedExtent = { codedWidth, codedHeight };
                _dpbResources[i].baseArrayLayer = 0;

                _dpbTextures[i]->TransitionLayout(
                    cmd,
                    VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
                    VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
                    VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR,
                    true
                );
            }

            VkVideoSessionCreateInfoKHR sessionInfo{ VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR };
            sessionInfo.pVideoProfile = &videoProfileInfo;
            sessionInfo.queueFamilyIndex = deviceObj->GetVideoDecodeQueue()->GetFamilyIndex();
            sessionInfo.maxCodedExtent = { codedWidth, codedHeight };
            sessionInfo.maxDpbSlots = _maxDpbSlots;
            sessionInfo.maxActiveReferencePictures = _maxDpbSlots - 1;
            sessionInfo.pictureFormat = data.format;
            sessionInfo.referencePictureFormat = data.format;
            sessionInfo.pStdHeaderVersion = &videoCapabilities.stdHeaderVersion;

            if (vkCreateVideoSessionKHR(device, &sessionInfo, nullptr, &_videoSession) == VK_SUCCESS) {

                uint32_t memReqCount = 0;
                vkGetVideoSessionMemoryRequirementsKHR(device, _videoSession, &memReqCount, nullptr);

                std::vector<VkVideoSessionMemoryRequirementsKHR> memReqs(memReqCount, { VK_STRUCTURE_TYPE_VIDEO_SESSION_MEMORY_REQUIREMENTS_KHR });
                vkGetVideoSessionMemoryRequirementsKHR(device, _videoSession, &memReqCount, memReqs.data());

                std::vector<VkBindVideoSessionMemoryInfoKHR> bindInfos(memReqCount, { VK_STRUCTURE_TYPE_BIND_VIDEO_SESSION_MEMORY_INFO_KHR });

                VkPhysicalDeviceMemoryProperties memProperties;
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

                for (uint32_t i = 0; i < memReqCount; ++i) {
                    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
                    allocInfo.allocationSize = memReqs[i].memoryRequirements.size;

                    for (uint32_t j = 0; j < memProperties.memoryTypeCount; j++) {
                        if ((memReqs[i].memoryRequirements.memoryTypeBits & (1 << j))) {
                            allocInfo.memoryTypeIndex = j;
                            break;
                        }
                    }

                    VkDeviceMemory mem = VK_NULL_HANDLE;
                    vkAllocateMemory(device, &allocInfo, nullptr, &mem);
                    _sessionMemories.push_back(mem);

                    bindInfos[i].memoryBindIndex = memReqs[i].memoryBindIndex;
                    bindInfos[i].memory = mem;
                    bindInfos[i].memoryOffset = 0;
                    bindInfos[i].memorySize = memReqs[i].memoryRequirements.size;
                }

                vkBindVideoSessionMemoryKHR(device, _videoSession, memReqCount, bindInfos.data());

                if (_parser && _parser->Parse(_extradata, sps, pps)) {
                    _spsId = sps.seq_parameter_set_id;
                    _ppsId = pps.pic_parameter_set_id;

                    VkVideoDecodeH264SessionParametersAddInfoKHR h264AddInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_ADD_INFO_KHR };
                    h264AddInfo.stdSPSCount = 1;
                    h264AddInfo.pStdSPSs = &sps;
                    h264AddInfo.stdPPSCount = 1;
                    h264AddInfo.pStdPPSs = &pps;

                    VkVideoDecodeH264SessionParametersCreateInfoKHR h264ParamsInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_SESSION_PARAMETERS_CREATE_INFO_KHR };
                    h264ParamsInfo.maxStdSPSCount = 1;
                    h264ParamsInfo.maxStdPPSCount = 1;
                    h264ParamsInfo.pParametersAddInfo = &h264AddInfo;

                    VkVideoSessionParametersCreateInfoKHR paramsInfo{ VK_STRUCTURE_TYPE_VIDEO_SESSION_PARAMETERS_CREATE_INFO_KHR };
                    paramsInfo.videoSessionParametersTemplate = VK_NULL_HANDLE;
                    paramsInfo.videoSession = _videoSession;
                    paramsInfo.pNext = &h264ParamsInfo;

                    vkCreateVideoSessionParametersKHR(device, &paramsInfo, nullptr, &_sessionParams);
                }
            }
        }

        uint32_t currentIndex = _frameIndex % _textures.size();
        _frameIndex++;
        auto currentTexture = _textures[currentIndex];

        if (_videoSession != VK_NULL_HANDLE && _sessionParams != VK_NULL_HANDLE)
        {
            SliceHeaderInfo sliceInfo = ParseAdvancedSliceHeader(data.bitstreamData, sps);

            bool isIdr = sliceInfo.isValid ? sliceInfo.isIdr : false;
            bool isIntra = sliceInfo.isValid ? sliceInfo.isIntra : false;
            bool isRef = sliceInfo.isValid ? sliceInfo.isReference : false;

            if (isIdr || _frameIndex == 1) {
                isIdr = true;
                for (auto& slot : _dpbSlots) slot.isActive = false;
                _currentDpbSlot = 0;
                _pocMsb = 0;
                _prevPocLsb = 0;
            }

            StdVideoDecodeH264PictureInfo syndPpsInfo{};
            syndPpsInfo.pic_parameter_set_id = _ppsId;
            syndPpsInfo.seq_parameter_set_id = _spsId;
            syndPpsInfo.flags.IdrPicFlag = isIdr ? 1 : 0;
            syndPpsInfo.flags.is_intra = isIntra ? 1 : 0;
            syndPpsInfo.flags.is_reference = isRef ? 1 : 0;
            syndPpsInfo.frame_num = sliceInfo.frameNum;

            if (sps.pic_order_cnt_type == STD_VIDEO_H264_POC_TYPE_0) {
                uint32_t maxPocLsb = 1 << (sps.log2_max_pic_order_cnt_lsb_minus4 + 4);
                int32_t currentPocMsb = _pocMsb;

                if (sliceInfo.picOrderCntLsb < _prevPocLsb && (_prevPocLsb - sliceInfo.picOrderCntLsb) >= maxPocLsb / 2) {
                    currentPocMsb += maxPocLsb;
                }
                else if (sliceInfo.picOrderCntLsb > _prevPocLsb && (sliceInfo.picOrderCntLsb - _prevPocLsb) > maxPocLsb / 2) {
                    currentPocMsb -= maxPocLsb;
                }

                syndPpsInfo.PicOrderCnt[0] = currentPocMsb + sliceInfo.picOrderCntLsb;
                syndPpsInfo.PicOrderCnt[1] = syndPpsInfo.PicOrderCnt[0];

                if (isRef) {
                    _prevPocLsb = sliceInfo.picOrderCntLsb;
                    _pocMsb = currentPocMsb;
                }
            }
            else {
                syndPpsInfo.PicOrderCnt[0] = sliceInfo.frameNum * 2;
                syndPpsInfo.PicOrderCnt[1] = syndPpsInfo.PicOrderCnt[0];
            }

            _dpbSlots[_currentDpbSlot].isActive = isRef;
            _dpbSlots[_currentDpbSlot].stdInfo.flags.top_field_flag = 0;
            _dpbSlots[_currentDpbSlot].stdInfo.flags.bottom_field_flag = 0;
            _dpbSlots[_currentDpbSlot].stdInfo.flags.used_for_long_term_reference = 0;
            _dpbSlots[_currentDpbSlot].stdInfo.flags.is_non_existing = 0;
            _dpbSlots[_currentDpbSlot].stdInfo.FrameNum = syndPpsInfo.frame_num;
            _dpbSlots[_currentDpbSlot].stdInfo.PicOrderCnt[0] = syndPpsInfo.PicOrderCnt[0];
            _dpbSlots[_currentDpbSlot].stdInfo.PicOrderCnt[1] = syndPpsInfo.PicOrderCnt[1];
            _dpbSlots[_currentDpbSlot].h264Info.pStdReferenceInfo = &_dpbSlots[_currentDpbSlot].stdInfo;
            _dpbSlots[_currentDpbSlot].slotInfo.pNext = &_dpbSlots[_currentDpbSlot].h264Info;
            _dpbSlots[_currentDpbSlot].slotInfo.slotIndex = _currentDpbSlot;
            _dpbSlots[_currentDpbSlot].slotInfo.pPictureResource = &_dpbResources[_currentDpbSlot];

            std::vector<VkVideoReferenceSlotInfoKHR> boundRefs;
            for (uint32_t i = 0; i < _maxDpbSlots; ++i) {
                if (_dpbSlots[i].isActive && i != _currentDpbSlot) {
                    boundRefs.push_back(_dpbSlots[i].slotInfo);
                }
            }

            VkVideoReferenceSlotInfoKHR setupBindInfo = _dpbSlots[_currentDpbSlot].slotInfo;
            setupBindInfo.slotIndex = -1;
            boundRefs.push_back(setupBindInfo);

            VkVideoBeginCodingInfoKHR beginInfo{ VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR };
            beginInfo.videoSession = _videoSession;
            beginInfo.videoSessionParameters = _sessionParams;
            beginInfo.referenceSlotCount = static_cast<uint32_t>(boundRefs.size());
            beginInfo.pReferenceSlots = boundRefs.empty() ? nullptr : boundRefs.data();

            vkCmdBeginVideoCodingKHR(cmd, &beginInfo);

            if (_frameIndex == 1) {
                VkVideoCodingControlInfoKHR controlInfo{ VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR };
                controlInfo.flags = VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR;
                vkCmdControlVideoCodingKHR(cmd, &controlInfo);
            }

            std::vector<VkVideoReferenceSlotInfoKHR> currentRefs;
            for (uint32_t i = 0; i < _maxDpbSlots; ++i) {
                if (_dpbSlots[i].isActive && i != _currentDpbSlot) {
                    currentRefs.push_back(_dpbSlots[i].slotInfo);
                }
            }

            VkVideoReferenceSlotInfoKHR setupSlot = _dpbSlots[_currentDpbSlot].slotInfo;
            setupSlot.slotIndex = _currentDpbSlot;

            std::vector<uint32_t> sliceOffsets;
            for (size_t i = 0; i + 3 < data.bitstreamData.size(); ) {
                uint32_t startCodeSize = 0;

                if (data.bitstreamData[i] == 0x00 && data.bitstreamData[i + 1] == 0x00 &&
                    data.bitstreamData[i + 2] == 0x01) {
                    startCodeSize = 3;
                }
                else if (i + 4 <= data.bitstreamData.size() &&
                    data.bitstreamData[i] == 0x00 && data.bitstreamData[i + 1] == 0x00 &&
                    data.bitstreamData[i + 2] == 0x00 && data.bitstreamData[i + 3] == 0x01) {
                    startCodeSize = 4;
                }

                if (startCodeSize > 0) {
                    uint8_t nalType = data.bitstreamData[i + startCodeSize] & 0x1F;
                    if (nalType >= 1 && nalType <= 5) {
                        sliceOffsets.push_back(static_cast<uint32_t>(i));
                    }
                    i += startCodeSize;
                }
                else {
                    i++;
                }
            }

            if (sliceOffsets.empty()) {
                sliceOffsets.push_back(0);
            }

            VkVideoDecodeH264PictureInfoKHR h264PicInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR };
            h264PicInfo.pStdPictureInfo = &syndPpsInfo;
            h264PicInfo.sliceCount = static_cast<uint32_t>(sliceOffsets.size());
            h264PicInfo.pSliceOffsets = sliceOffsets.data();

            VkVideoDecodeInfoKHR decodeInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR };
            decodeInfo.pNext = &h264PicInfo;
            decodeInfo.srcBuffer = result.bitstreamBuffer->Handle();
            decodeInfo.srcBufferOffset = 0;
            decodeInfo.srcBufferRange = byteSize;
            decodeInfo.dstPictureResource = _dpbResources[_currentDpbSlot];
            decodeInfo.pSetupReferenceSlot = &setupSlot;
            decodeInfo.referenceSlotCount = static_cast<uint32_t>(currentRefs.size());
            decodeInfo.pReferenceSlots = currentRefs.empty() ? nullptr : currentRefs.data();

            vkCmdDecodeVideoKHR(cmd, &decodeInfo);

            VkVideoEndCodingInfoKHR endInfo{ VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR };
            vkCmdEndVideoCodingKHR(cmd, &endInfo);

            _dpbTextures[_currentDpbSlot]->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_READ_BIT,
                false
            );

            currentTexture->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                true
            );

            std::vector<VkImageCopy> copyRegions(2);
            copyRegions[0].srcSubresource = { VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1 };
            copyRegions[0].dstSubresource = { VK_IMAGE_ASPECT_PLANE_0_BIT, 0, 0, 1 };
            copyRegions[0].extent = { _width, _height, 1 };

            copyRegions[1].srcSubresource = { VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1 };
            copyRegions[1].dstSubresource = { VK_IMAGE_ASPECT_PLANE_1_BIT, 0, 0, 1 };
            copyRegions[1].extent = { _width / 2, _height / 2, 1 };

            vkCmdCopyImage(cmd,
                _dpbTextures[_currentDpbSlot]->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                currentTexture->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                2, copyRegions.data());

            _dpbTextures[_currentDpbSlot]->TransitionLayout(
                cmd,
                VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
                VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
                VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR | VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR,
                false
            );

            if (isRef) {
                _currentDpbSlot = (_currentDpbSlot + 1) % _maxDpbSlots;
                _dpbSlots[_currentDpbSlot].isActive = false;
            }
        }

        uploader->RegisterImageTransfer({
             .image = currentTexture->Handle(),
             .aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT,
             .mipLevels = 1,
             .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
             .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            });

        currentTexture->OverrideInternalState(
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT
        );

        _reorderQueue[data.pts] = currentTexture;

        if (_reorderQueue.size() >= 4) {
            auto it = _reorderQueue.begin();
            result.texture = it->second;
            _reorderQueue.erase(it);
            result.isFrameReady = true;
        }
        else {
            result.texture = nullptr;
            result.isFrameReady = false;
        }

        return result;
    }
}