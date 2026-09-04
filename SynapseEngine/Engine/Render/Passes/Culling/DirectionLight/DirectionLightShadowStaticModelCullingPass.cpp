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

#include "DirectionLightShadowStaticModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowCullingPC.glsl"

    bool DirectionLightShadowStaticModelCullingPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        auto settings = context.scene->GetSettings();

        return settings->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU 
            && settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh
            && pool && pool->Size() > 0;
    }

    void DirectionLightShadowStaticModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("DirectionLightShadowStaticModelCullingProgram", {
            ShaderNames::DirectionLightShadowStaticModelCullingComp
            }, config);
    }

    void DirectionLightShadowStaticModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        uint32_t activeChunks = drawData->Chunks.chunkCounter.load(std::memory_order_relaxed);
        if (activeChunks == 0) return;

        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<DirectionLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->isStaticPhase = 1;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowStaticModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t pyramidIdx = context.frameIndex;
        auto depthPyramid = context.scene->GetSceneDrawData()->DirectionLightShadow.shadowDepthPyramid[pyramidIdx].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void DirectionLightShadowStaticModelCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();

        if (drawData->Chunks.chunkCounter.load(std::memory_order_relaxed) == 0) return;

        uint32_t fIdx = context.frameIndex;

        VkBuffer dispatchBuf = drawData->DirectionLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);
        vkCmdDispatchIndirect(context.cmd, dispatchBuf, 0);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->DirectionLightShadow.modelDispatchBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = compManager->GetComponentBuffer(BufferNames::DirectionLightShadowModelVisibleData, fIdx).buffer->Handle();
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
    }
}