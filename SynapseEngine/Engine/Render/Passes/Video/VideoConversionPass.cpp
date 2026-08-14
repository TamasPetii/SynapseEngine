// (C)2026 Tamás Péter
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

#include "VideoConversionPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Video/VideoManager.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"

namespace Syn {

    void VideoConversionPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("YuvToRgbProgram", {
            ShaderNames::YuvToRgbComp
            });
    }

    bool VideoConversionPass::ShouldExecute(const RenderContext& context) const {
        return true;
    }

    void VideoConversionPass::PrepareFrame(const RenderContext& context) {
        auto videoManager = ServiceLocator::Get<VideoManager>();
        auto activeVideos = videoManager->GetResourceSnapshot();

        _activeTargets.clear();
        _imageTransitions.clear();

        std::vector<std::pair<uint32_t, VkImageView>> bindlessUpdates;

        for (uint32_t i = 0; i < activeVideos.size(); ++i) {
            const auto& snapshot = activeVideos[i];

            if (snapshot.state == ResourceState::Ready && snapshot.resource && snapshot.resource->video && snapshot.resource->video->image) {

                auto yuvImage = snapshot.resource->video->image;

                if (yuvImage->GetFormat() != VK_FORMAT_G8_B8R8_2PLANE_420_UNORM) {
                    continue;
                }

                if (!snapshot.resource->video->convertedImage) {
                    Vk::ImageConfig rgbConfig{};
                    rgbConfig.width = yuvImage->GetConfig().width;
                    rgbConfig.height = yuvImage->GetConfig().height;
                    rgbConfig.depth = 1;
                    rgbConfig.format = VK_FORMAT_R8G8B8A8_UNORM;
                    rgbConfig.mipLevels = 1;
                    rgbConfig.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                    snapshot.resource->video->convertedImage = std::make_shared<Vk::Image>(rgbConfig);

                    bindlessUpdates.push_back({ i, snapshot.resource->video->convertedImage->GetView(Vk::ImageViewNames::Default) });
                }

                ActiveVideoTarget target;
                target.sourceYuv = yuvImage;
                target.targetRgba = snapshot.resource->video->convertedImage;
                target.lumaView = yuvImage->GetView(Vk::ImageViewNames::Luma);
                target.chromaView = yuvImage->GetView(Vk::ImageViewNames::Chroma);
                target.width = yuvImage->GetConfig().width;
                target.height = yuvImage->GetConfig().height;

                _activeTargets.push_back(target);
            }
        }

        if (!bindlessUpdates.empty()) {
            videoManager->UpdateVideoBindlessBatch(bindlessUpdates);
        }
    }

    void VideoConversionPass::Dispatch(const RenderContext& context) {
        if (_activeTargets.empty()) return;

        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);

        for (const auto& target : _activeTargets) {
            VkImageLayout currentYuvLayout = target.sourceYuv->GetLayout();
            if (currentYuvLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                Vk::ImageBarrierInfo yuvBarrier{};
                yuvBarrier.image = target.sourceYuv->Handle();
                yuvBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                yuvBarrier.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT;
                yuvBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                yuvBarrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
                yuvBarrier.oldLayout = currentYuvLayout;
                yuvBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                yuvBarrier.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
                yuvBarrier.baseMipLevel = 0;
                yuvBarrier.levelCount = 1;
                yuvBarrier.baseArrayLayer = 0;
                yuvBarrier.layerCount = 1;

                Vk::ImageUtils::InsertBarrier(context.cmd, yuvBarrier);

                target.sourceYuv->OverrideInternalState(
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_READ_BIT
                );
            }

            VkImageLayout currentRgbaLayout = target.targetRgba->GetLayout();
            if (currentRgbaLayout != VK_IMAGE_LAYOUT_GENERAL) {
                Vk::ImageBarrierInfo rgbaBarrier{};
                rgbaBarrier.image = target.targetRgba->Handle();
                rgbaBarrier.srcStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                rgbaBarrier.srcAccess = VK_ACCESS_2_MEMORY_READ_BIT;
                rgbaBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                rgbaBarrier.dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
                rgbaBarrier.oldLayout = currentRgbaLayout;
                rgbaBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                rgbaBarrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                rgbaBarrier.baseMipLevel = 0;
                rgbaBarrier.levelCount = 1;
                rgbaBarrier.baseArrayLayer = 0;
                rgbaBarrier.layerCount = 1;

                Vk::ImageUtils::InsertBarrier(context.cmd, rgbaBarrier);

                target.targetRgba->OverrideInternalState(
                    VK_IMAGE_LAYOUT_GENERAL,
                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                    VK_ACCESS_2_SHADER_WRITE_BIT
                );
            }

            Vk::PushDescriptorWriter pushWriter;

            pushWriter.AddCombinedImageSampler(0, target.lumaView, sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            pushWriter.AddCombinedImageSampler(1, target.chromaView, sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            pushWriter.AddStorageImage(2, target.targetRgba->GetView(Vk::ImageViewNames::Default), VK_IMAGE_LAYOUT_GENERAL);

            pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(target.width, ComputeGroupSize::Image8D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(target.height, ComputeGroupSize::Image8D);

            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = target.targetRgba->Handle();
            barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = 0;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;

            Vk::ImageUtils::InsertBarrier(context.cmd, barrier);

            target.targetRgba->OverrideInternalState(
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_SHADER_READ_BIT
            );
        }
    }
}