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

#include "PointLightShadowHizDownsamplePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include <glm/glm.hpp>
#include <algorithm>
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Scene/DrawData/PointLightShadowDrawGroup.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/HizDownSamplePC.glsl"

    bool PointLightShadowHizDownsamplePass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();
        return pool && pool->Size() > 0;
    }

    void PointLightShadowHizDownsamplePass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowHizDownsampleProgram", {
            ShaderNames::HizDownsample
            });
    }

    void PointLightShadowHizDownsamplePass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto& depthPyramid = drawData->PointLightShadow.shadowDepthPyramid[context.frameIndex];

        _imageTransitions.push_back({
            depthPyramid.get(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            false
            });
    }

    void PointLightShadowHizDownsamplePass::Dispatch(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        auto drawData = context.scene->GetSceneDrawData();
        auto& depthPyramid = drawData->PointLightShadow.shadowDepthPyramid[context.frameIndex];

        uint32_t mipLevels = depthPyramid->GetConfig().mipLevels;
        glm::vec2 currentInSize = glm::vec2(POINT_SHADOW_ATLAS_SIZE, POINT_SHADOW_ATLAS_SIZE);

        Vk::PushDescriptorWriter pushWriter;

        // Skip 0 -> PointLightShadowHizCopyPass already done it!
        for (uint32_t i = 1; i < mipLevels; ++i) {

            glm::vec2 currentOutSize = glm::vec2(
                std::max(1.0f, std::floor(currentInSize.x / 2.0f)),
                std::max(1.0f, std::floor(currentInSize.y / 2.0f))
            );

            std::string parentMipName = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i - 1);
            std::string currentMipName = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i);

            pushWriter.AddCombinedImageSampler(
                0,
                depthPyramid->GetView(parentMipName),
                sampler->Handle(),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.AddStorageImage(
                1,
                depthPyramid->GetView(currentMipName),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            Vk::PushConstant<HizDownSamplePC> pc;
            pc->inImageSize = currentInSize;
            pc->outImageSize = currentOutSize;
            pc.Push(context.cmd, _shaderProgram->GetLayout());

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.x, ComputeGroupSize::Image16D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.y, ComputeGroupSize::Image16D);

            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = depthPyramid->Handle();
            barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = i;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;

            Vk::ImageUtils::InsertBarrier(context.cmd, barrier);

            currentInSize = currentOutSize;
        }

        depthPyramid->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}