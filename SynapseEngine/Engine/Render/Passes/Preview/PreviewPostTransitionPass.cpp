#include "PreviewPostTransitionPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/PreviewManager.h"

namespace Syn {

    void PreviewPostTransitionPass::PrepareFrame(const RenderContext& context) {
        auto pm = ServiceLocator::GetPreviewManager();

        _imageTransitions.push_back({
            .image = pm->GetAtlasImage(),
            .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccess = VK_ACCESS_2_SHADER_READ_BIT,
            .discardContent = false
            });
    }
}