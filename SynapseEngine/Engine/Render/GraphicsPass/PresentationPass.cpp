#include "PresentationPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn {

    void PresentationPass::Initialize() {
        _useDynamicRendering = false;
        _shaderProgram = nullptr;
    }

    void PresentationPass::PrepareFrame(const RenderContext& context) 
    {
        auto vkContext = ServiceLocator::GetVkContext();
        auto swapChain = vkContext->GetSwapChain();

        auto image = swapChain->GetImage(context.swapchainImageIndex);
        VkExtent2D extent = swapChain->GetExtent();

        _imageTransitions.clear();

        _imageTransitions.push_back({
            .image = image,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .dstStage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            .dstAccess = VK_ACCESS_2_NONE,
            .discardContent = false
            });
    }
}