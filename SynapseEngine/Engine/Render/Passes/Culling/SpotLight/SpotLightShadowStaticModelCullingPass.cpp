#include "SpotLightShadowStaticModelCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/SpotLightShadowCullingPC.glsl"

    bool SpotLightShadowStaticModelCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightComponent>();
        return context.scene->GetSettings()->culling.spotLightShadowCullingDevice == CullingDeviceType::GPU
            && context.scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh
            && pool && pool->Size() > 0;
    }

    void SpotLightShadowStaticModelCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("SpotLightShadowStaticModelCullingProgram", {
            ShaderNames::SpotLightShadowStaticModelCullingComp
            }, config);
    }

    void SpotLightShadowStaticModelCullingPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        _shouldDispatch = true;

        Vk::PushConstant<SpotLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowStaticModelCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->SpotLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(0, depthPyramid->GetView(Vk::ImageViewNames::Default), maxSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void SpotLightShadowStaticModelCullingPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        VkBuffer cullBuffer = drawData->SpotLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);

        Vk::BufferBarrierInfo readyBarrier{};
        readyBarrier.buffer = cullBuffer;
        readyBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        readyBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        readyBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        readyBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, readyBarrier);

        vkCmdDispatchIndirect(context.cmd, cullBuffer, 0);
    }
}