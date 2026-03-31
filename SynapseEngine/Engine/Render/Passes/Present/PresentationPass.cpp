#include "PresentationPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Image/ImageUtils.h"

namespace Syn {
    void PresentationPass::PrepareFrame(const RenderContext& context) 
    {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto image = swapChain->GetImage(context.swapchainImageIndex);

        _imageTransitions.push_back({
            .image = image,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .dstStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .dstAccess = VK_ACCESS_2_NONE
            });
    }
}