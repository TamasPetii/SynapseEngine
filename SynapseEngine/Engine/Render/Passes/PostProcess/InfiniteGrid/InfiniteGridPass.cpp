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

#include "InfiniteGridPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/InfiniteGridPC.glsl"

    bool InfiniteGridPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->debug.enableInfiniteGrid;
    }

    void InfiniteGridPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("InfiniteGridProgram", {
            ShaderNames::FullscreenVert,
            ShaderNames::InfiniteGridFrag
            }, config);

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .lineWidth = 1.0f
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_FALSE,
                .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
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

    void InfiniteGridPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::OpaqueDepth)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void InfiniteGridPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto settings = scene->GetSettings();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<InfiniteGridPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);

        uint32_t planeFlags = 0;
        if (settings->debug.gridShowXZ) planeFlags |= (1 << 0);
        if (settings->debug.gridShowXY) planeFlags |= (1 << 1);
        if (settings->debug.gridShowYZ) planeFlags |= (1 << 2);
        pc->planeFlags = planeFlags;

        uint32_t axisFlags = 0;
        if (settings->debug.gridShowAxisX) axisFlags |= (1 << 0);
        if (settings->debug.gridShowAxisY) axisFlags |= (1 << 1);
        if (settings->debug.gridShowAxisZ) axisFlags |= (1 << 2);
        pc->axisFlags = axisFlags;

        pc->gridScale = settings->debug.gridScale;
        pc->fadeDistance = settings->debug.fadeDistance;
        pc->gridThickness = settings->debug.gridThickness;
        pc->axisThickness = settings->debug.axisThickness;

        pc->gridColor = settings->debug.gridColor;
        pc->axisXColor = settings->debug.axisXColor;
        pc->axisYColor = settings->debug.axisYColor;
        pc->axisZColor = settings->debug.axisZColor;

        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void InfiniteGridPass::Draw(const RenderContext& context) {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}