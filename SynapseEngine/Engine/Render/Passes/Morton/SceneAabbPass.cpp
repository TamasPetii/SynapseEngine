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

#include "SceneAabbPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void SceneAabbPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("StaticSceneAABBProgram", { 
            ShaderNames::StaticSceneAABB
        }, config);
    }

    bool SceneAabbPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();

        bool isEnabled = context.scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh;

        if (!isEnabled || !pool || pool->GetStorage().GetStaticEntities().empty()) {
            _wasEnabled = false;
            return false;
        }

        uint32_t currentVersion = context.scene->GetSceneDrawData()->Chunks.mortonRebuildVersion.load(std::memory_order_relaxed);

        if (!_wasEnabled || _lastMortonVersion != currentVersion) {
            _needsRebuild = true;
            _lastMortonVersion = currentVersion;
        }
        _wasEnabled = true;

        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();
        return hasDirty || _needsRebuild || (_countdown > 0);
    }

    void SceneAabbPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ChunkBuilderPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SceneAabbPass::Dispatch(const RenderContext& context) {
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
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();
        uint32_t fIdx = context.frameIndex;

        struct {
            uint32_t min[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
            uint32_t max[3] = { 0x00000000, 0x00000000, 0x00000000 };
        } resetData;

        VkBuffer aabbBufferHandle = drawData->Chunks.sceneAabbBuffer.GetHandle(fIdx);

        Vk::BufferUpdateInfo resetInfo{};
        resetInfo.buffer = aabbBufferHandle;
        resetInfo.offset = 0;
        resetInfo.size = sizeof(resetData);
        resetInfo.pData = &resetData;
        Vk::BufferUtils::UpdateBuffer(context.cmd, resetInfo);

        Vk::BufferBarrierInfo resetBarrier{};
        resetBarrier.buffer = aabbBufferHandle;
        resetBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        resetBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        resetBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        resetBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, resetBarrier);

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo aabbBarrier{};
        aabbBarrier.buffer = aabbBufferHandle;
        aabbBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbBarrier);
    }
}