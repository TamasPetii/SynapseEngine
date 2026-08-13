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

#include "ChunkBuilderPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void ChunkBuilderPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ChunkBuilderProgram", { 
            ShaderNames::ChunkBuilder 
        }, config);
    }

    bool ChunkBuilderPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();

        bool isEnabled = context.scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh;

        if (!isEnabled || !pool || pool->GetStorage().GetStaticEntities().empty()) {
            _wasEnabled = false;
            return false;
        }

        if (!_wasEnabled) {
            _needsRebuild = true;
        }

        _wasEnabled = true;

        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();
        return hasDirty || _needsRebuild || (_countdown > 0);
    }

    void ChunkBuilderPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        Vk::PushConstant<ChunkBuilderPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ChunkBuilderPass::Dispatch(const RenderContext& context) {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();

        if (hasDirty || _needsRebuild) {
            _countdown = context.framesInFlight;
            _needsRebuild = false;
        }

        if (_countdown > 0) {
            _countdown--;
        }

        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        auto drawGroup = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        VkDrawIndirectCommand drawTemplate = drawGroup->Chunks.wireframeCmdTemplate;

        Vk::BufferUpdateInfo drawUpdateInfo{};
        drawUpdateInfo.buffer = drawGroup->Chunks.mortonIndirectDrawBuffer.GetHandle(fIdx);
        drawUpdateInfo.offset = 0;
        drawUpdateInfo.size = sizeof(VkDrawIndirectCommand);
        drawUpdateInfo.pData = &drawTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, drawUpdateInfo);
            
        Vk::BufferBarrierInfo drawBarrier{};
        drawBarrier.buffer = drawUpdateInfo.buffer;
        drawBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        drawBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        drawBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, drawBarrier);

        VkDispatchIndirectCommand dispatchTemplate = drawGroup->Chunks.dispatchCmdTemplate;

        Vk::BufferUpdateInfo dispatchUpdateInfo{};
        dispatchUpdateInfo.buffer = drawGroup->Chunks.mortonIndirectDispatchBuffer.GetHandle(fIdx);
        dispatchUpdateInfo.offset = 0;
        dispatchUpdateInfo.size = sizeof(VkDispatchIndirectCommand);
        dispatchUpdateInfo.pData = &dispatchTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, dispatchUpdateInfo);

        Vk::BufferBarrierInfo dispatchBarrier{};
        dispatchBarrier.buffer = dispatchUpdateInfo.buffer;
        dispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        dispatchBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        dispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, dispatchBarrier);

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, 32);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo chunkDataBarrier{};
        chunkDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonChunkData, fIdx).buffer->Handle();
        chunkDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkDataBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkDataBarrier);

        Vk::BufferBarrierInfo chunkTransformIndicesBarrier{};
        chunkTransformIndicesBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonChunkTransformsIndex, fIdx).buffer->Handle();
        chunkTransformIndicesBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkTransformIndicesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkTransformIndicesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkTransformIndicesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkTransformIndicesBarrier);

        Vk::BufferBarrierInfo indirectDispatchBarrier{};
        indirectDispatchBarrier.buffer = drawGroup->Chunks.mortonIndirectDispatchBuffer.GetHandle(fIdx);
        indirectDispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectDispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectDispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectDispatchBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectDispatchBarrier);

        Vk::BufferBarrierInfo indirectDrawBarrier{};
        indirectDrawBarrier.buffer = drawGroup->Chunks.mortonIndirectDrawBuffer.GetHandle(fIdx);
        indirectDrawBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectDrawBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectDrawBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectDrawBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectDrawBarrier);
    }
}