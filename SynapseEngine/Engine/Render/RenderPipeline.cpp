#include "RenderPipeline.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn
{
    void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
    {
        _passes.push_back(std::move(pass));
    }

    void RenderPipeline::InitializeAll()
    {
        for (auto& pass : _passes) {
            pass->Initialize();
        }
    }

    void RenderPipeline::Execute(const RenderContext& context)
    {
        if (context.scene)
        {
            for (auto& pass : _passes) {
                if (pass->ShouldExecute(context))
                {
                    pass->Execute(context);
                }
            }
        }

        auto image = ServiceLocator::GetVkContext()->GetSwapChain()->GetImage(context.swapchainImageIndex);
		image->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, false);
    }
}