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

#include "DirectionLightShadowModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowCullingPC.glsl"

    bool DirectionLightShadowModelCullingPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        return context.scene->GetSettings()->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU 
            && pool && pool->Size() > 0;
    }

    void DirectionLightShadowModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("DirectionLightShadowModelCullingProgram", {
            ShaderNames::DirectionLightShadowModelCullingComp
            }, config);
    }

    void DirectionLightShadowModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto settings = scene->GetSettings();
        auto transformPool = scene->GetRegistry()->GetPool<TransformComponent>();
        auto lightPool = scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();

        if (!transformPool || transformPool->Size() == 0 || !lightPool || lightPool->Size() == 0) {
            _totalModelsToTest = 0;
            _activeLights = 0;
            return;
        }

        uint32_t totalCount = static_cast<uint32_t>(transformPool->Size());
        uint32_t staticCount = static_cast<uint32_t>(transformPool->GetStorage().GetStaticEntities().size());
        _activeLights = context.scene->GetSceneDrawData()->DirectionLightShadow.visibleLightCount;

        bool bvhEnabled = (settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh ||
                           settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh);

        if (_isStaticPhase) {
            _totalModelsToTest = bvhEnabled ? 0 : staticCount;
        }
        else {
            _totalModelsToTest = totalCount;
        }

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<DirectionLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc->isStaticPhase = _isStaticPhase ? 1 : 0;
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t pyramidIdx = _isStaticPhase ? ((context.frameIndex + context.framesInFlight - 1) % context.framesInFlight) : context.frameIndex;
        auto depthPyramid = context.scene->GetSceneDrawData()->DirectionLightShadow.shadowDepthPyramid[pyramidIdx].get();
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

    void DirectionLightShadowModelCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalModelsToTest == 0 || _activeLights == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        // 3D Grid Dispatch: X = Dynamics, Y = Lights, Z = Cascades
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalModelsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, _activeLights, CASCADES_PER_LIGHT);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->DirectionLightShadow.modelDispatchBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo listBarrier{};
        listBarrier.buffer = compManager->GetComponentBuffer(BufferNames::DirectionLightShadowModelVisibleData, fIdx).buffer->Handle();
        listBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        listBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        listBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, listBarrier);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->DirectionLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->DirectionLightShadow.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);
    }
}