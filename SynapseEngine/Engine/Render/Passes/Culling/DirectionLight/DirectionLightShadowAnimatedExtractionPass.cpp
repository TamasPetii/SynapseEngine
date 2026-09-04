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

#include "DirectionLightShadowAnimatedExtractionPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Manager/ComponentBufferManager.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowCullingPC.glsl"

    bool DirectionLightShadowAnimatedExtractionPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        return context.scene->GetSettings()->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void DirectionLightShadowAnimatedExtractionPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("DirectionLightShadowAnimatedExtractionProgram", {
            ShaderNames::DirectionLightShadowAnimatedExtractionComp
            }, config);
    }

    void DirectionLightShadowAnimatedExtractionPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<DirectionLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->dataSource = 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowAnimatedExtractionPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto transformPool = scene->GetRegistry()->GetPool<TransformComponent>();

        if (!transformPool) return;

        uint32_t staticCount = static_cast<uint32_t>(transformPool->GetStorage().GetStaticEntities().size());
        if (staticCount == 0) return;

        uint32_t dispatchCount = std::max(1u, ComputeGroupSize::CalculateDispatchCount(staticCount, ComputeGroupSize::Buffer256D));
        vkCmdDispatch(context.cmd, dispatchCount, 1, 1);

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        VkBuffer dispatchBuffer = drawData->DirectionLightShadow.animatedStaticDispatchBuffer.GetHandle(fIdx);
        Vk::BufferBarrierInfo dispatchBarrier{};
        dispatchBarrier.buffer = dispatchBuffer;
        dispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        dispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        dispatchBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, dispatchBarrier);

        VkBuffer listBuffer = compManager->GetComponentBuffer(BufferNames::DirectionLightShadowAnimatedStaticList, fIdx).buffer->Handle();
        if (listBuffer != VK_NULL_HANDLE) {
            Vk::BufferBarrierInfo listBarrier{};
            listBarrier.buffer = listBuffer;
            listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
        }
    }
}