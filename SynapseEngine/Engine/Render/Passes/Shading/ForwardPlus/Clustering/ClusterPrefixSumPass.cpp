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

#include "ClusterPrefixSumPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn 
{
    #include "Engine/Shaders/Includes/PushConstants/ClusterPrefixSumPC.glsl"

    void ClusterPrefixSumPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ClusterPrefixSumProgram", {
            ShaderNames::ClusterPrefixSum
            }, config);
    }

    void ClusterPrefixSumPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ClusterPrefixSumPC> pc;
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ClusterPrefixSumPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        Vk::BufferBarrierInfo listBarrierPre{};
        listBarrierPre.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(context.frameIndex);
        listBarrierPre.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrierPre.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrierPre.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrierPre.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrierPre);

        VkBuffer indirectBuffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(context.frameIndex);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, offsetof(ForwardPlusDispatchArgs, prefixSum));
        
        Vk::BufferBarrierInfo listBarrierPost{};
        listBarrierPost.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(context.frameIndex);
        listBarrierPost.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrierPost.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrierPost.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrierPost.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrierPost);
    }
}