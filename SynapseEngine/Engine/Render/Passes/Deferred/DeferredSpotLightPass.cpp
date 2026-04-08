#include "DeferredSpotLightPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/SamplerNames.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DeferredSpotLightPC.glsl"

    bool DeferredSpotLightPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableDeferredSpotLights;
    }

    void DeferredSpotLightPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("DeferredSpotLightProgram", {
            ShaderNames::DeferredSpotLightVert,
            ShaderNames::DeferredSpotLightFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_FRONT_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void DeferredSpotLightPass::PrepareFrame(const RenderContext& context) {
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
            .colorAttachments = _colorAttachments,
            .depthAttachment = nullptr,
            .layerCount = 1
        };
    }

    void DeferredSpotLightPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto bufferManager = scene->GetComponentBufferManager();
        auto modelManager = ServiceLocator::GetModelManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        auto pyramid = modelManager->GetResource(MeshSourceNames::Pyramid);

        DeferredSpotLightPC pc{};
        pc.cameraBufferAddr = bufferManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = bufferManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.spotLightDataAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightData, fIdx);
        pc.spotLightSparseMapAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightSparseMap, fIdx);
        pc.spotLightColliderDataAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);
        pc.visibleLightAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        pc.spotLightShadowSparseMapAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightShadowSparseMap, fIdx);
        pc.spotLightShadowDataAddr = bufferManager->GetBufferAddr(BufferNames::SpotLightShadowData, fIdx);

        pc.vertexPositionsAddr = pyramid->hardwareBuffers.vertexPositions->GetDeviceAddress();
        pc.indicesAddr = pyramid->hardwareBuffers.indices->GetDeviceAddress();

        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.screenWidth = (float)_graphicsState.renderArea->width;
        pc.screenHeight = (float)_graphicsState.renderArea->height;

        vkCmdPushConstants(
            context.cmd,
            _shaderProgram->GetLayout(),
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(DeferredSpotLightPC),
            &pc
        );
    }

    void DeferredSpotLightPass::BindDescriptors(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge)->Handle();

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(0, group->GetImage(RenderTargetNames::ColorMetallic)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(1, group->GetImage(RenderTargetNames::NormalRoughness)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(2, group->GetImage(RenderTargetNames::Depth)->GetView(Vk::ImageViewNames::Default), sampler, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void DeferredSpotLightPass::Draw(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        auto indirectBuffer = drawData->spotLightIndirectCommandBuffers[context.frameIndex]->Handle();

        vkCmdDrawIndirect(
            context.cmd,
            indirectBuffer,
            0,
            1,
            sizeof(VkDrawIndirectCommand)
        );
    }
}