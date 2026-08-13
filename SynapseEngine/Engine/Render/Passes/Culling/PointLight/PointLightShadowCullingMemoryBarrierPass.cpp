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

#include "PointLightShadowCullingMemoryBarrierPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"

namespace Syn {
    void PointLightShadowCullingMemoryBarrierPass::Execute(const RenderContext& context) {
        auto scene = context.scene;

        auto pool = scene->GetRegistry()->GetPool<PointLightComponent>();
        if (scene->GetSettings()->culling.pointLightShadowCullingDevice != CullingDeviceType::GPU || !pool || pool->Size() == 0) {
            return;
        }

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        // Barrier for Radix Sort keys
        Vk::BufferBarrierInfo sortKeysBarrier{};
        sortKeysBarrier.buffer = drawData->PointLightShadow.drawCallKeyBuffer.GetHandle(fIdx);
        sortKeysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortKeysBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        sortKeysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortKeysBarrier);

        // Barrier for unsorted payload instances
        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->PointLightShadow.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);

        // Barrier for mesh count tracking
        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        // Barrier for indirect commands
        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->PointLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);
    }
}