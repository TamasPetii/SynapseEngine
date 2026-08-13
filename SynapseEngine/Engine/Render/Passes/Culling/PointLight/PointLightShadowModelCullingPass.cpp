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

#include "PointLightShadowModelCullingPass.h"
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
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightShadowCullingPC.glsl"

    bool PointLightShadowModelCullingPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();
        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void PointLightShadowModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowModelCullingProgram", {
            ShaderNames::PointLightShadowModelCullingComp
            }, config);
    }

    void PointLightShadowModelCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto transformPool = scene->GetRegistry()->GetPool<TransformComponent>();
        auto lightPool = scene->GetRegistry()->GetPool<PointLightComponent>();

        if (!transformPool || transformPool->Size() == 0 || !lightPool || lightPool->Size() == 0) {
            _shouldDispatch = false;
            return;
        }

        bool useBvh = scene->GetSettings()->culling.pointLightShadowSpatialAcceleration != SpatialAccelerationType::None;
        uint32_t totalTrans = static_cast<uint32_t>(transformPool->Size());
        uint32_t staticTrans = static_cast<uint32_t>(transformPool->GetStaticEntities().size());

        // Skip static entities if BVH is managing them
        _totalModelsToTest = useBvh ? (totalTrans - staticTrans) : totalTrans;

        if (_totalModelsToTest == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<PointLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->PointLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void PointLightShadowModelCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (!_shouldDispatch) return;

        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        bool isPointCullingGpu = scene->GetSettings()->culling.pointLightCullingDevice == CullingDeviceType::GPU;

        VkBuffer cullBuffer = drawData->PointLightShadow.modelCullingIndirectDispatchBuffer.GetHandle(fIdx);
        VkBuffer countBuffer = drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);

        // Setup the compute dispatch structure
        VkDispatchIndirectCommand cmd{};
        cmd.x = ComputeGroupSize::CalculateDispatchCount(_totalModelsToTest, ComputeGroupSize::Buffer32D);
        cmd.y = isPointCullingGpu ? 0 : drawData->PointLightShadow.visibleLightCount;
        cmd.z = 1;

        Vk::BufferUpdateInfo updateInfo{};
        updateInfo.buffer = cullBuffer;
        updateInfo.offset = 0;
        updateInfo.size = sizeof(VkDispatchIndirectCommand);
        updateInfo.pData = &cmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

        if (isPointCullingGpu) {
            // Wait for the buffer update
            Vk::BufferBarrierInfo updateBarrier{};
            updateBarrier.buffer = cullBuffer;
            updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            updateBarrier.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);

            // Copy dynamic light count from the earlier Light culling pass into dispatch y component
            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = countBuffer;
            copyInfo.dstBuffer = cullBuffer;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = offsetof(VkDispatchIndirectCommand, y);
            copyInfo.size = sizeof(uint32_t);
            Vk::BufferUtils::CopyBuffer(context.cmd, copyInfo);
        }

        // Ready the buffer for indirect read
        Vk::BufferBarrierInfo readyBarrier{};
        readyBarrier.buffer = cullBuffer;
        readyBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        readyBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        readyBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        readyBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, readyBarrier);

        vkCmdDispatchIndirect(context.cmd, cullBuffer, 0);
    }
}