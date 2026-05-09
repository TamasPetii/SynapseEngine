#include "DeferredDirectionLightPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DeferredDirectionLightPC.glsl"

    bool DeferredDirectionLightPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->pipelineType == PipelineType::Deferred 
            && context.scene->GetSettings()->enableDeferredDirectionalLights
            && !context.scene->GetSettings()->enableDebugVisibility;
    }

    void DeferredDirectionLightPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("DeferredDirectionLightProgram", {
            ShaderNames::DeferredDirectionLightVert,
            ShaderNames::DeferredDirectionLightFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL
            },
            .depth = {
                .testEnable = VK_FALSE,
                .writeEnable = VK_FALSE
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ONE
                }
            },
            .colorAttachmentCount = 1
        };
    }

    void DeferredDirectionLightPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        auto mainImg = group->GetImage(RenderTargetNames::Main);

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = mainImg->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _renderInfo = Vk::RenderingInfoConfig{ 
            .renderArea = extent, 
            .colorAttachments = _colorAttachments
        };
    }

    void DeferredDirectionLightPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto bufferManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        uint32_t fIdx = context.frameIndex;

        DeferredDirectionLightPC pc{};
		pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(DeferredDirectionLightPC), &pc);
    }

    void DeferredDirectionLightPass::BindDescriptors(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge)->Handle();

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(0, group->GetImage(RenderTargetNames::ColorMetallic)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(1, group->GetImage(RenderTargetNames::NormalRoughness)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(2, group->GetImage(RenderTargetNames::OpaqueDepth)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void DeferredDirectionLightPass::Draw(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
		auto fIdx = context.frameIndex;
		auto isGpu = context.scene->GetSettings()->enableGpuCulling;

		auto indirectBuffer = drawData->DirectionLights.indirectBuffer.GetHandle(fIdx, isGpu);

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            1,
            sizeof(VkDrawIndirectCommand)
        );
    }
}