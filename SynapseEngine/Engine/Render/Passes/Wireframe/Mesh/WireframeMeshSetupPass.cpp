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

#include "WireframeMeshSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeSetupPC.glsl"

    bool WireframeMeshSetupPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->debug.enableWireframeMeshSphere 
            || context.scene->GetSettings()->debug.enableWireframeMeshAabb;
    }

    void WireframeMeshSetupPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("WireframeSetupProgram", {
            ShaderNames::WireframeSetup
            });
    }

    void WireframeMeshSetupPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        if (totalCommands == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<WireframeSetupPC> pc{};
		pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void WireframeMeshSetupPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(totalCommands, ComputeGroupSize::Buffer256D);

        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo aabbBarrier{};
        aabbBarrier.buffer = drawData->Debug.modelAabbIndirectBuffer.GetHandle(fIdx);
        aabbBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        aabbBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbBarrier);

        Vk::BufferBarrierInfo sphereBarrier{};
        sphereBarrier.buffer = drawData->Debug.modelSphereIndirectBuffer.GetHandle(fIdx);
        sphereBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sphereBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sphereBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        sphereBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sphereBarrier);
    }
}