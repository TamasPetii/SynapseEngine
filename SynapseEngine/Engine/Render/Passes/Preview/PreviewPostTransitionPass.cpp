#include "PreviewPostTransitionPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/PreviewManager.h"

namespace Syn {

    void PreviewPostTransitionPass::PrepareFrame(const RenderContext& context) {
        auto pm = ServiceLocator::GetPreviewManager();

        _imageTransitions.push_back({
            .image = pm->GetAtlasImage(),
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = pm->GetScratchColorImage(),
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });

        _imageTransitions.push_back({
            .image = pm->GetScratchBloomImage(),
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT | VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });
    }
}