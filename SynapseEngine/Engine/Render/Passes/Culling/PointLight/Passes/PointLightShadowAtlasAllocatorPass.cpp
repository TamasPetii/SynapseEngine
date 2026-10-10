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

#include "PointLightShadowAtlasAllocatorPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightShadowCullingPC.glsl"

    void PointLightShadowAtlasAllocatorPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowAtlasAllocatorProgram", {
            ShaderNames::PointLightShadowAtlasAllocatorComp
            }, { .useDescriptorBuffers = false });
    }

    bool PointLightShadowAtlasAllocatorPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightShadowComponent>();
        return context.scene->GetSettings()->culling.pointLightCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void PointLightShadowAtlasAllocatorPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<PointLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowAtlasAllocatorPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        vkCmdDispatch(context.cmd, 1, 1, 1);

        Vk::BufferBarrierInfo shadowDataBarrier{};
        shadowDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightShadowData, fIdx).buffer->Handle();
        shadowDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        shadowDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        shadowDataBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        shadowDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, shadowDataBarrier);

        Vk::BufferBarrierInfo gridLookupBarrier{};
        gridLookupBarrier.buffer = scene->GetSceneDrawData()->PointLightShadow.gridLookupBuffer.GetHandle(fIdx);
        gridLookupBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        gridLookupBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        gridLookupBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        gridLookupBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, gridLookupBarrier);
    }
}