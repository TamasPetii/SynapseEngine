#include "OpaqueDepthTransitionPrepass.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    bool OpaqueDepthTransitionPrepass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.pipelineType == PipelineType::ForwardPlus;
    }

    void OpaqueDepthTransitionPrepass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        if (auto entityImg = group->GetImage(RenderTargetNames::EntityIndex)) {
            if (entityImg->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = entityImg,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                    .discardContent = false
                    });
            }
        }

        if (auto entityMsaaImg = group->GetImage(RenderTargetNames::EntityIndexMSAA)) {
            if (entityMsaaImg->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = entityMsaaImg,
                    .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                    .discardContent = false
                    });
            }
        }

        if (auto depthImg = group->GetImage(RenderTargetNames::OpaqueDepth)) {
            if (depthImg->GetLayout() != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = depthImg,
                    .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    .discardContent = false
                    });
            }
        }

        if (auto depthMsaaImg = group->GetImage(RenderTargetNames::OpaqueDepthMSAA)) {
            if (depthMsaaImg->GetLayout() != VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = depthMsaaImg,
                    .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    .discardContent = false
                    });
            }
        }
       
    }
}