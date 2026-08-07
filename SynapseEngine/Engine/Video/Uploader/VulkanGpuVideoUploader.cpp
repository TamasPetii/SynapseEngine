#include "VulkanGpuVideoUploader.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn
{
    VulkanGpuVideoUploader::VulkanGpuVideoUploader(uint32_t width, uint32_t height)
        : _width(width), _height(height) {}

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
    }

    VideoUploadResult VulkanGpuVideoUploader::Upload(const GpuVideoPacket& data, VkCommandBuffer cmd)
    {
        VideoUploadResult result;
        result.isFrameReady = false;

        if (data.bitstreamData.empty()) {
            return result;
        }

        size_t byteSize = data.bitstreamData.size();
        result.bitstreamBuffer = Vk::BufferFactory::CreateStaging(byteSize);
        result.bitstreamBuffer->Write(data.bitstreamData.data(), byteSize, 0);

        if (!_texture) {
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

            VkSamplerYcbcrConversionCreateInfo ycbcrInfo{ VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_CREATE_INFO };
            ycbcrInfo.format = data.format;
            ycbcrInfo.ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
            ycbcrInfo.ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
            ycbcrInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
            ycbcrInfo.xChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
            ycbcrInfo.yChromaOffset = VK_CHROMA_LOCATION_MIDPOINT;
            ycbcrInfo.chromaFilter = VK_FILTER_LINEAR;
            ycbcrInfo.forceExplicitReconstruction = VK_FALSE;

            auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();

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

            _texture = std::make_shared<Vk::Image>(imgConfig);
        }

        _texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
            VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR,
            VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR,
            true
        );

        if (_videoSession != VK_NULL_HANDLE) {
            VkVideoBeginCodingInfoKHR beginInfo{ VK_STRUCTURE_TYPE_VIDEO_BEGIN_CODING_INFO_KHR };
            beginInfo.videoSession = _videoSession;
            beginInfo.videoSessionParameters = _sessionParams;

            vkCmdBeginVideoCodingKHR(cmd, &beginInfo);

            VkVideoDecodeInfoKHR decodeInfo{ VK_STRUCTURE_TYPE_VIDEO_DECODE_INFO_KHR };
            decodeInfo.srcBuffer = result.bitstreamBuffer->Handle();
            decodeInfo.srcBufferOffset = 0;
            decodeInfo.srcBufferRange = byteSize;

            VkVideoPictureResourceInfoKHR dstPictureResource{ VK_STRUCTURE_TYPE_VIDEO_PICTURE_RESOURCE_INFO_KHR };
            dstPictureResource.imageViewBinding = _texture->GetView();
            decodeInfo.dstPictureResource = dstPictureResource;

            vkCmdDecodeVideoKHR(cmd, &decodeInfo);

            VkVideoEndCodingInfoKHR endInfo{ VK_STRUCTURE_TYPE_VIDEO_END_CODING_INFO_KHR };
            vkCmdEndVideoCodingKHR(cmd, &endInfo);
        }

        _texture->TransitionLayout(
            cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            VK_ACCESS_2_NONE,
            false
        );

        _texture->OverrideInternalState(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        result.texture = _texture;
        result.isFrameReady = true;
        return result;
    }
}