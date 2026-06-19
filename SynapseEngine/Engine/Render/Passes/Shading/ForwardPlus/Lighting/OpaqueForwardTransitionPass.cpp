#include "OpaqueForwardTransitionPass.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {

    bool OpaqueForwardTransitionPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->lighting.pipelineType == PipelineType::ForwardPlus 
            && !context.scene->GetSettings()->debug.enableDebugVisibility;;
    }

    void OpaqueForwardTransitionPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        if (auto mainImg = group->GetImage(RenderTargetNames::Main)) {
            if (mainImg->GetLayout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                _imageTransitions.push_back({
                    .image = mainImg,
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
    }
}