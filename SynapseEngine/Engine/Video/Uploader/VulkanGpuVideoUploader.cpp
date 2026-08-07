#include "VulkanGpuVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Logger/SynLog.h"

#include <vk_video/vulkan_video_codecs_common.h>
#include <vk_video/vulkan_video_codec_h264std.h>
#include <vk_video/vulkan_video_codec_h264std_decode.h>

namespace Syn
{
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

    VideoUploadResult VulkanGpuVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd)
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

        VkVideoDecodeH264ProfileInfoKHR h264ProfileInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR };
        h264ProfileInfo.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_MAIN;
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
            VkSamplerYcbcrConversionCreateInfo ycbcrInfo{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO };
            ycbcrInfo.format = data.format;
            ycbcrInfo.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
            ycbcrInfo.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
            ycbcrInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            ycbcrInfo.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
            ycbcrInfo.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
            ycbcrInfo.chromaFilter = VK_FILTER_LINEAR;
            ycbcrInfo.forceExplicitReconstruction = VK_FALSE;

            if (_ycbcrConversion == VK_NULL_HANDLE) {
                vkCreateSamplerYcbcrConversion(device, &ycbcrInfo, nullptr, &_ycbcrConversion);
            }

            Vk::ImageConfig imgConfig{};
            imgConfig.width = _width;
            imgConfig.height = _height;
            imgConfig.depth = 1;
            imgConfig.format = data.format;
            imgConfig.mipLevels = 1;
            imgConfig.usage = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR | VK_IMAGE_USAGE_SAMPLED_BIT;
            imgConfig.videoProfileList = &videoProfileList;
            imgConfig.ycbcrConversion = _ycbcrConversion;

            for (auto& tex : _textures) {
                tex = std::make_shared<Vk::Image>(imgConfig);
            }

            VkVideoDecodeH264CapabilitiesKHR h264Capabilities{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR };
            VkVideoDecodeCapabilitiesKHR decodeCapabilities{ VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR };
            decodeCapabilities.pNext = &h264Capabilities;
            VkVideoCapabilitiesKHR videoCapabilities{ VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR };
            videoCapabilities.pNext = &decodeCapabilities;

            vkGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, &videoProfileInfo, &videoCapabilities);

            VkVideoSessionCreateInfoKHR sessionInfo{ VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR };
            sessionInfo.pVideoProfile = &videoProfileInfo;
            sessionInfo.queueFamilyIndex = deviceObj->GetVideoDecodeQueue()->GetFamilyIndex();
            sessionInfo.maxCodedExtent = { _width, _height };
            sessionInfo.maxDpbSlots = 0;
            sessionInfo.maxActiveReferencePictures = 0;
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

                StdVideoH264SequenceParameterSet sps{};
                StdVideoH264PictureParameterSet pps{};

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
                else {
                    Error("Failed to parse H264 extradata in VulkanGpuVideoUploader.");
                }
            }
        }

        uint32_t currentIndex = _frameIndex % _textures.size();
        _frameIndex++;
        auto currentTexture = _textures[currentIndex];

        currentTexture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
            VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
            VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
            true
        );

        if (_videoSession != VK_NULL_HANDLE && _sessionParams != VK_NULL_HANDLE) {
            VkVideoBeginCodingInfoKHR beginInfo{ VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR };
            beginInfo.videoSession = _videoSession;
            beginInfo.videoSessionParameters = _sessionParams;

            vkCmdBeginVideoCodingKHR(cmd, &beginInfo);

            if (_frameIndex == 1) {
                VkVideoCodingControlInfoKHR controlInfo{ VK_STRUCTURE_TYPE_VIDEO_CODING_CONTROL_INFO_KHR };
                controlInfo.flags = VK_VIDEO_CODING_CONTROL_RESET_BIT_KHR;
                vkCmdControlVideoCodingKHR(cmd, &controlInfo);
            }

            StdVideoDecodeH264PictureInfo syndPpsInfo{};
            syndPpsInfo.pic_parameter_set_id = _ppsId;
            syndPpsInfo.seq_parameter_set_id = _spsId;
            syndPpsInfo.flags.IdrPicFlag = 1;
            syndPpsInfo.flags.is_intra = 1;
            syndPpsInfo.flags.is_reference = 0;
            syndPpsInfo.frame_num = static_cast<uint16_t>(_frameIndex & 0xFFFF);
            syndPpsInfo.PicOrderCnt[0] = static_cast<int32_t>(_frameIndex * 2);
            syndPpsInfo.PicOrderCnt[1] = 0;

            uint32_t sliceOffset = 0;

            VkVideoDecodeH264PictureInfoKHR h264PicInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PICTURE_INFO_KHR };
            h264PicInfo.pStdPictureInfo = &syndPpsInfo;
            h264PicInfo.sliceCount = 1;
            h264PicInfo.pSliceOffsets = &sliceOffset;

            VkVideoDecodeInfoKHR decodeInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR };
            decodeInfo.pNext = &h264PicInfo;
            decodeInfo.srcBuffer = result.bitstreamBuffer->Handle();
            decodeInfo.srcBufferOffset = 0;
            decodeInfo.srcBufferRange = byteSize;

            VkVideoPictureResourceInfoKHR dstPictureResource{ VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR };
            dstPictureResource.imageViewBinding = currentTexture->GetView();
            dstPictureResource.codedExtent = { _width, _height };
            decodeInfo.dstPictureResource = dstPictureResource;

            vkCmdDecodeVideoKHR(cmd, &decodeInfo);

            VkVideoEndCodingInfoKHR endInfo{ VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR };
            vkCmdEndVideoCodingKHR(cmd, &endInfo);
        }

        currentTexture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_NONE,
            false
        );

        currentTexture->OverrideInternalState(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        result.texture = currentTexture;
        result.isFrameReady = true;
        return result;
    }
}