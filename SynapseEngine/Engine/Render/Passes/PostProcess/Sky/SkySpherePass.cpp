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

#include "SkySpherePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Manager/DescriptorManager.h"
#include "Engine/Vk/Descriptor/DescriptorUtils.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SkySpherePC.glsl"

    SkySpherePass::SkySpherePass() {}

    bool SkySpherePass::ShouldExecute(const RenderContext& context) const
    {
        const auto& envSettings = context.scene->GetSettings()->environment;
        bool is2DMode = (envSettings.skyMode == SkyMode::EquirectangularTexture || envSettings.skyMode == SkyMode::OctahedralTexture);
        return envSettings.enableSky && is2DMode && envSettings.activeEnvironmentId != UINT32_MAX && !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void SkySpherePass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = true;
        config.layoutOverride = [descriptorManager](uint32_t setIndex) {
            if (setIndex == 0) {
                return descriptorManager->GetBindlessLayout();
            }
            return VkDescriptorSetLayout{};
            };

        _shaderProgramId = shaderManager->LoadProgramAsync("SkySphereProgram", {
            ShaderNames::SkyVert,
            ShaderNames::SkySphereFrag
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

    void SkySpherePass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };
        _graphicsState.renderArea = extent;

        _colorAttachments.clear();
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

    void SkySpherePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t fIdx = context.frameIndex;

        const auto& envSettings = scene->GetSettings()->environment;

        Vk::PushConstant<SkySpherePC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->environmentIndex = envSettings.activeEnvironmentId;
        pc->mappingType = (envSettings.skyMode == SkyMode::EquirectangularTexture) ? 0 : 1;

        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SkySpherePass::BindDescriptors(const RenderContext& context)
    {
        auto descriptorManager = ServiceLocator::Get<DescriptorManager>();
        std::vector<std::pair<uint32_t, Vk::DescriptorBuffer*>> buffersToBind;

        if (auto descBuffer = descriptorManager->GetBindlessBuffer()) {
            buffersToBind.push_back({ 0, descBuffer });
        }

        Vk::DescriptorUtils::BindMultipleBuffer(context.cmd, _shaderProgram->GetLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, buffersToBind);
    }

    void SkySpherePass::Draw(const RenderContext& context)
    {
        vkCmdDraw(context.cmd, 3, 1, 0, 0);
    }
}