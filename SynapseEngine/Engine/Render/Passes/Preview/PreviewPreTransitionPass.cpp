#include "PreviewPreTransitionPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/PreviewManager.h"

namespace Syn {

    void PreviewPreTransitionPass::PrepareFrame(const RenderContext& context) {
        auto pm = ServiceLocator::GetPreviewManager();

        _imageTransitions.push_back({
            .image = pm->GetAtlasImage(),
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .discardContent = false
            });
    }
}