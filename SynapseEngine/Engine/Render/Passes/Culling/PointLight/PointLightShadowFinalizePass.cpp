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

#include "PointLightShadowFinalizePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightShadowCullingPC.glsl"

    void PointLightShadowFinalizePass::Initialize() {
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowFinalizeProgram", {
            ShaderNames::PointLightShadowFinalizeComp
            }, config);
    }

    bool PointLightShadowFinalizePass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightShadowComponent>();
        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void PointLightShadowFinalizePass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<PointLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowFinalizePass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        // Issue the indirect dispatch configured by SetupPass
        VkBuffer finalizeCmdBuffer = drawData->PointLightShadow.finalizeDispatchBuffer.GetHandle(fIdx);
        vkCmdDispatchIndirect(context.cmd, finalizeCmdBuffer, 0);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->PointLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);

        Vk::BufferBarrierInfo descBarrier{};
        descBarrier.buffer = drawData->PointLightShadow.descriptorBuffer.GetHandle(fIdx);
        descBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        descBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        descBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        descBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, descBarrier);

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->PointLightShadow.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);
    }
}