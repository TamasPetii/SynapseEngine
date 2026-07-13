#include "SsaoInitPass.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    void SsaoInitPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        if (!group) return;

        auto depthPyramid = group->GetImage(RenderTargetNames::DepthPyramid);
        auto ssaoAo = group->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoAoInt = group->GetImage(RenderTargetNames::SsaoAoIntermediate); 

        _imageTransitions.push_back({
            depthPyramid,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            ssaoAo,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            true
            });

        _imageTransitions.push_back({
            ssaoAoInt,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            true
            });
    }
}