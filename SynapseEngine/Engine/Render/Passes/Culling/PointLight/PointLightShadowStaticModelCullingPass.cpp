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

#include "PointLightShadowStaticModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/PointLightShadowCullingPC.glsl"

    bool PointLightShadowStaticModelCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightShadowComponent>();
        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
            && context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh
            && pool && pool->Size() > 0;
    }

    void PointLightShadowStaticModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowStaticModelCullingProgram", {
            ShaderNames::PointLightShadowStaticModelCullingComp
            }, config);
    }

    void PointLightShadowStaticModelCullingPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        _shouldDispatch = true;

        Vk::PushConstant<PointLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowStaticModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->PointLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(0, depthPyramid->GetView(Vk::ImageViewNames::Default), maxSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void PointLightShadowStaticModelCullingPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        // Use the indirect dispatch buffer populated by the Static Chunk pass
        VkBuffer cullBuffer = drawData->PointLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);

        // Wait for the chunk culling to finish writing the dynamic dispatch count
        Vk::BufferBarrierInfo readyBarrier{};
        readyBarrier.buffer = cullBuffer;
        readyBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        readyBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        readyBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        readyBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, readyBarrier);

        vkCmdDispatchIndirect(context.cmd, cullBuffer, 0);
    }
}