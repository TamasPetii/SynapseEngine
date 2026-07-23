#include "PointLightShadowMortonChunkCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/PointLightShadowCullingPC.glsl"

    bool PointLightShadowMortonChunkCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();
        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
            && context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            && pool && pool->Size() > 0;
    }

    void PointLightShadowMortonChunkCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowMortonChunkCullingProgram", {
            ShaderNames::PointLightShadowMortonChunkCullingComp
            }, config);
    }

    void PointLightShadowMortonChunkCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto transformPool = scene->GetRegistry()->GetPool<TransformComponent>();

        _staticCount = static_cast<uint32_t>(transformPool->GetStaticEntities().size());

        if (_staticCount == 0) {
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

    void PointLightShadowMortonChunkCullingPass::BindDescriptors(const RenderContext& context) {
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

    void PointLightShadowMortonChunkCullingPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isPointCullingGpu = context.scene->GetSettings()->culling.pointLightCullingDevice == CullingDeviceType::GPU;

        VkBuffer cullBuffer = drawData->PointLightShadow.modelCullingIndirectDispatchBuffer.GetHandle(fIdx);
        VkBuffer countBuffer = drawData->PointLightShadow.visibleCountDispatchBuffer.GetHandle(fIdx);

        VkDispatchIndirectCommand cmd{};
        cmd.x = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        cmd.y = isPointCullingGpu ? 0 : drawData->PointLightShadow.visibleLightCount;
        cmd.z = 1;

        Vk::BufferUpdateInfo updateInfo{};
        updateInfo.buffer = cullBuffer;
        updateInfo.offset = 0;
        updateInfo.size = sizeof(VkDispatchIndirectCommand);
        updateInfo.pData = &cmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

        if (isPointCullingGpu) {
            Vk::BufferBarrierInfo updateBarrier{};
            updateBarrier.buffer = cullBuffer;
            updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            updateBarrier.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);

            Vk::BufferCopyInfo copyInfo{};
            copyInfo.srcBuffer = countBuffer;
            copyInfo.dstBuffer = cullBuffer;
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = offsetof(VkDispatchIndirectCommand, y);
            copyInfo.size = sizeof(uint32_t);
            Vk::BufferUtils::CopyBuffer(context.cmd, copyInfo);
        }

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