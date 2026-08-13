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

#include "PointLightShadowBufferResetPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {
    void PointLightShadowBufferResetPass::Execute(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        bool isPointCullingGpu = context.scene->GetSettings()->culling.pointLightCullingDevice == CullingDeviceType::GPU;

        if (isPointCullingGpu) {
            Vk::BufferFillInfo fillBase{};
            fillBase.buffer = drawData->PointLights.indirectBuffer.GetHandle(fIdx);
            fillBase.offset = sizeof(uint32_t);
            fillBase.size = sizeof(uint32_t);
            fillBase.data = 0;
            Vk::BufferUtils::FillBuffer(context.cmd, fillBase);

            Vk::BufferFillInfo fillShadow{};
            fillShadow.buffer = drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);
            fillShadow.offset = 0;
            fillShadow.size = sizeof(uint32_t);
            fillShadow.data = 0;
            Vk::BufferUtils::FillBuffer(context.cmd, fillShadow);

            Vk::BufferBarrierInfo fillShadowBarrier{};
            fillShadowBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            fillShadowBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            fillShadowBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            fillShadowBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

            fillShadowBarrier.buffer = fillBase.buffer;
            fillShadowBarrier.size = fillBase.size;
            fillShadowBarrier.offset = fillBase.offset;
            Vk::BufferUtils::InsertBarrier(context.cmd, fillShadowBarrier);

            fillShadowBarrier.buffer = fillShadow.buffer;
            fillShadowBarrier.size = fillShadow.size;
            fillShadowBarrier.offset = fillShadow.offset;
            Vk::BufferUtils::InsertBarrier(context.cmd, fillShadowBarrier);
        }

        // 1. Reset mesh count
        Vk::BufferFillInfo fillMesh{};
        fillMesh.buffer = drawData->PointLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);
        fillMesh.offset = 0;
        fillMesh.size = sizeof(uint32_t);
        fillMesh.data = 0;
        Vk::BufferUtils::FillBuffer(context.cmd, fillMesh);

        // 2. Reset finalize setup
        VkDispatchIndirectCommand finalizeCmd{ 0, 1, 1 };
        Vk::BufferUpdateInfo updateFinalize{};
        updateFinalize.buffer = drawData->PointLightShadow.finalizeDispatchBuffer.GetHandle(fIdx);
        updateFinalize.offset = 0;
        updateFinalize.size = sizeof(VkDispatchIndirectCommand);
        updateFinalize.pData = &finalizeCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateFinalize);

        // 3. Reset static chunk dispatch count
        VkDispatchIndirectCommand zeroCmd{ 0, 1, 1 };
        Vk::BufferUpdateInfo updateStaticChunk{};
        updateStaticChunk.buffer = drawData->PointLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);
        updateStaticChunk.offset = 0;
        updateStaticChunk.size = sizeof(VkDispatchIndirectCommand);
        updateStaticChunk.pData = &zeroCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateStaticChunk);

        // 4. Reset morton chunk dispatch count
        Vk::BufferUpdateInfo updateMortonChunk{};
        updateMortonChunk.buffer = drawData->PointLightShadow.mortonChunkDispatchBuffer.GetHandle(fIdx);
        updateMortonChunk.offset = 0;
        updateMortonChunk.size = sizeof(VkDispatchIndirectCommand);
        updateMortonChunk.pData = &zeroCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateMortonChunk);

        // 5. Issue barriers for all resets
        Vk::BufferBarrierInfo alwaysBarrier{};
        alwaysBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        alwaysBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        alwaysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        alwaysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

        alwaysBarrier.buffer = fillMesh.buffer;
        alwaysBarrier.size = fillMesh.size;
        alwaysBarrier.offset = fillMesh.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);

        alwaysBarrier.buffer = updateFinalize.buffer;
        alwaysBarrier.size = updateFinalize.size;
        alwaysBarrier.offset = updateFinalize.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);

        alwaysBarrier.buffer = updateStaticChunk.buffer;
        alwaysBarrier.size = updateStaticChunk.size;
        alwaysBarrier.offset = updateStaticChunk.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);

        alwaysBarrier.buffer = updateMortonChunk.buffer;
        alwaysBarrier.size = updateMortonChunk.size;
        alwaysBarrier.offset = updateMortonChunk.offset;
        Vk::BufferUtils::InsertBarrier(context.cmd, alwaysBarrier);
    }
}