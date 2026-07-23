#include "DirectionLightShadowMortonChunkCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
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
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowCullingPC.glsl"

    void DirectionLightShadowMortonChunkCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("DirectionLightShadowMortonChunkCullingProgram", {
            ShaderNames::DirectionLightShadowMortonChunkCullingComp
            }, config);
    }

    bool DirectionLightShadowMortonChunkCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        auto lightPool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        auto settings = context.scene->GetSettings();

        return settings->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU 
            && settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh 
            && pool && !pool->GetStorage().GetStaticEntities().empty() && lightPool && lightPool->Size() > 0;
    }

    void DirectionLightShadowMortonChunkCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());
        _activeLights = static_cast<uint32_t>(scene->GetRegistry()->GetPool<DirectionLightShadowComponent>()->Size());

        Vk::PushConstant<DirectionLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowMortonChunkCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->DirectionLightShadow.shadowDepthPyramid[prevFrameIndex].get();
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

    void DirectionLightShadowMortonChunkCullingPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0 || _activeLights == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        //Todo: Direction Light Culling and indirect dispatch

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, _activeLights, CASCADES_PER_LIGHT);

        Vk::BufferBarrierInfo visibleIndexBarrier{};
        visibleIndexBarrier.buffer = scene->GetComponentBufferManager()->GetComponentBuffer(BufferNames::DirectionLightShadowMortonChunkVisibleIndex, fIdx).buffer->Handle();
        visibleIndexBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleIndexBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleIndexBarrier);

        Vk::BufferBarrierInfo dispatchBarrier{};
        dispatchBarrier.buffer = drawData->DirectionLightShadow.mortonChunkDispatchBuffer.GetHandle(fIdx);
        dispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        dispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, dispatchBarrier);
    }
}