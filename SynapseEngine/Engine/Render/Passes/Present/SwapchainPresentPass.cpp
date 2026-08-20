#include "SwapchainPresentPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Render/RenderNames.h"

namespace Syn 
{
    void SwapchainPresentPass::PrepareFrame(const RenderContext& context) {
        auto vkContext = ServiceLocator::Get<Vk::Context>();
        auto swapchainImage = vkContext->GetSwapChain()->GetImage(context.swapchainImageIndex);

        _imageTransitions.push_back({
            .image = swapchainImage,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .dstStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .dstAccess = VK_ACCESS_2_NONE,
            .discardContent = false
            });
    }
}