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

#include "GuiPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"
#include <vector>
#include <string>

namespace Syn 
{
    void GuiPass::Initialize() {
        _useDynamicRendering = true;
    }

    void GuiPass::PrepareFrame(const RenderContext& context) {
        if (!context.onRenderGui) return;

        auto vkContext = ServiceLocator::Get<Vk::Context>();
        auto swapChain = vkContext->GetSwapChain();

        auto swapchainImage = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = { swapchainImage->GetExtent().width, swapchainImage->GetExtent().height };

        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        std::vector<std::string> debugTargets = {
            RenderTargetNames::Main,
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo,
            RenderTargetNames::EntityIndex,
            RenderTargetNames::DepthPyramid,
            RenderTargetNames::Bloom,
            RenderTargetNames::OpaqueDepth,
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal
        };

        for (const auto& targetName : debugTargets) {
            auto image = group->GetImage(targetName);
            if (image) {
                _imageTransitions.push_back({
                    .image = image,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
                    .discardContent = false
                    });
            }
        }

        _imageTransitions.push_back({
            .image = swapchainImage,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = true
            });

        VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 1.0f}} };

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = swapchainImage->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .clearValue = clearColor,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = nullptr,
            .layerCount = 1
        };
    }

    void GuiPass::Execute(const RenderContext& context) {
        if (!context.onRenderGui) return;

        _colorAttachments.clear();
        _imageTransitions.clear();

        PrepareFrame(context);

        for (const auto& transition : _imageTransitions) {
            transition.image->TransitionLayout(
                context.cmd,
                transition.newLayout,
                transition.dstStage,
                transition.dstAccess,
                transition.discardContent
            );
        }

        if (_useDynamicRendering && _renderInfo.has_value()) {
            Vk::RenderUtils::BeginRendering(context.cmd, _renderInfo.value());
        }

        context.onRenderGui(context.cmd);

        if (_useDynamicRendering && _renderInfo.has_value()) {
            Vk::RenderUtils::EndRendering(context.cmd);
        }
    }
}