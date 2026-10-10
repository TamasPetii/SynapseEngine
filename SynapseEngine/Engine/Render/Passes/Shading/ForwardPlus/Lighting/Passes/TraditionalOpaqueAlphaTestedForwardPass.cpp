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

#include "TraditionalOpaqueAlphaTestedForwardPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Video/VideoManager.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/TraditionalMeshletPassPC.glsl"

    bool TraditionalOpaqueAlphaTestedForwardPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->lighting.pipelineType == PipelineType::ForwardPlus
            && !context.scene->GetSettings()->debug.enableDebugVisibility;;
    }

    TraditionalOpaqueAlphaTestedForwardPass::TraditionalOpaqueAlphaTestedForwardPass(MaterialRenderType renderType)
        : _renderType(renderType)
    {
        assert(_renderType == MaterialRenderType::AlphaTestedOpaque1Sided || _renderType == MaterialRenderType::AlphaTestedOpaque2Sided);

        if (_renderType == MaterialRenderType::AlphaTestedOpaque1Sided) {
            _passName = "TraditionalOpaqueAlphaTestedForward1Sided";
        }
        else {
            _passName = "TraditionalOpaqueAlphaTestedForward2Sided";
        }
    }

    void TraditionalOpaqueAlphaTestedForwardPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto videoManager = ServiceLocator::Get<VideoManager>();
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.defines = { ShaderDefines::EnableAlphaTest };
        config.layoutOverride = [descriptorManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return descriptorManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };


        _shaderProgramId = shaderManager->LoadProgramAsync("TraditionalOpaqueAlphaTestedForwardProgram", {
            ShaderNames::TraditionalVert,
            ShaderNames::OpaqueForwardFrag
            }, config);

        VkCullModeFlags cullMode = (_renderType == MaterialRenderType::AlphaTestedOpaque2Sided) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = cullMode,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_EQUAL
            },
            .blendStates = {
                {
                .enable = VK_FALSE,
                .srcColorFactor = VK_BLEND_FACTOR_ONE,
                .dstColorFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD
                }
            },
            .colorAttachmentCount = 1,
            .renderArea = std::nullopt
        };
    }

    void TraditionalOpaqueAlphaTestedForwardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        uint32_t msaaSamples = context.scene->GetSettings()->lighting.msaaSamples;
        _graphicsState.raster.samples = static_cast<VkSampleCountFlagBits>(msaaSamples);

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::MainMSAA)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .resolveImageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
            .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::OpaqueDepthMSAA)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .resolveMode = VK_RESOLVE_MODE_NONE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void TraditionalOpaqueAlphaTestedForwardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<TraditionalMeshletPassPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->baseDescriptorOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        pc->materialRenderType = static_cast<uint32_t>(_renderType);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void TraditionalOpaqueAlphaTestedForwardPass::BindDescriptors(const RenderContext& context)
    {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto drawData = context.scene->GetSceneDrawData();

        uint fIdx = context.frameIndex;
        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto prevRtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, prevFrameIndex);
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, fIdx);

        auto ssaoTexture = rtGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoSampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

        auto dirShadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx].get();
        auto pointShadowAtlas = drawData->PointLightShadow.shadowAtlas[fIdx].get();
        auto spotShadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx].get();
        auto shadowSampler = imageManager->GetSampler(SamplerNames::ShadowSampler);

        auto dirColorAtlas = drawData->DirectionLightShadow.shadowColorAtlas[fIdx].get();
        auto pointColorAtlas = drawData->PointLightShadow.shadowColorAtlas[fIdx].get();
        auto spotColorAtlas = drawData->SpotLightShadow.shadowColorAtlas[fIdx].get();
        auto linearSampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);
        auto nearestSampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        auto staticDirShadowAtlas = drawData->DirectionLightShadow.staticShadowAtlas[fIdx].get();
        auto staticDirColorAtlas = drawData->DirectionLightShadow.staticShadowColorAtlas[fIdx].get();

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(1, ssaoTexture->GetView(Vk::ImageViewNames::Default), ssaoSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        pushWriter.AddCombinedImageSampler(2, dirShadowAtlas->GetView(Vk::ImageViewNames::Default), shadowSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(3, pointShadowAtlas->GetView(Vk::ImageViewNames::Default), shadowSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(4, spotShadowAtlas->GetView(Vk::ImageViewNames::Default), shadowSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        pushWriter.AddCombinedImageSampler(5, dirColorAtlas->GetView(Vk::ImageViewNames::Default), nearestSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(6, pointColorAtlas->GetView(Vk::ImageViewNames::Default), nearestSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(7, spotColorAtlas->GetView(Vk::ImageViewNames::Default), nearestSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        pushWriter.AddCombinedImageSampler(8, staticDirShadowAtlas->GetView(Vk::ImageViewNames::Default), shadowSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriter.AddCombinedImageSampler(9, staticDirColorAtlas->GetView(Vk::ImageViewNames::Default), nearestSampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);

        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;
        if (auto descBuffer = descriptorManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, descBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void TraditionalOpaqueAlphaTestedForwardPass::Draw(const RenderContext& context)
    {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        auto indirectBuffer = drawData->Models.indirectBuffer.GetHandle(context.frameIndex);
        auto countBuffer = drawData->Models.drawCountBuffer.GetHandle(context.frameIndex);

        uint32_t commandOffset = drawData->Models.traditionalCmdOffsets[_renderType];
        uint32_t maxCommandCount = drawData->Models.traditionalCmdCounts[_renderType];

        if (maxCommandCount > 0) {
            VkDeviceSize countBufferOffset = _renderType * sizeof(uint32_t);

            vkCmdDrawIndirectCount(
                context.cmd,
                indirectBuffer,
                commandOffset * sizeof(VkDrawIndirectCommand),
                countBuffer,
                countBufferOffset,
                maxCommandCount,
                sizeof(VkDrawIndirectCommand)
            );
        }
    }
}