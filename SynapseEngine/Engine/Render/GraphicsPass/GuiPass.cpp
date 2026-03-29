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
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto swapchainImage = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = { swapchainImage->GetExtent().width, swapchainImage->GetExtent().height };

        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        std::vector<std::string> debugTargets = {
            RenderTargetNames::Main,
            RenderTargetNames::ColorMetallic,
            RenderTargetNames::NormalRoughness,
            RenderTargetNames::EmissiveAo,
            RenderTargetNames::EntityIndex,
            RenderTargetNames::DepthPyramid,
            RenderTargetNames::Bloom,
            RenderTargetNames::Depth,
            RenderTargetNames::TransparentAccum,
            RenderTargetNames::TransparentReveal,
            RenderTargetNames::DebugTopologyPipeline,
            RenderTargetNames::DebugMeshletLod,
			RenderTargetNames::DebugMaterialUv
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
        if (!context.onRenderGui || !context.scene) return;

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