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

#include "GeometryMortonModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    void GeometryMortonModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("GeometryMortonModelCullingProgram", {
            ShaderNames::GeometryMortonModelCullingComp
            }, config);
    }

    bool GeometryMortonModelCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();

        return context.scene->GetSettings()->culling.geometryCullingDevice == CullingDeviceType::GPU
            && context.scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh
            && pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void GeometryMortonModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        Vk::PushConstant<ModelMeshCullingPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryMortonModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
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

    void GeometryMortonModelCullingPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        VkBuffer indirectBuffer = drawData->Chunks.mortonChunkVisibleIndirectDispatchBuffer.GetHandle(fIdx);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, 0);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->Models.computeCountBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = compManager->GetComponentBuffer(BufferNames::ModelVisibleData, fIdx).buffer->Handle();
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);
    }
}