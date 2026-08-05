#include "RenderPipeline.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Profiler/IGpuProfiler.h"
#include "Engine/Statistics/IRenderStatCollector.h"
#include "Engine/Shader/ShaderManager.h"

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
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        if (shaderManager->IsCompiling())
        {
            auto image = ServiceLocator::Get<Vk::Context>()->GetSwapChain()->GetImage(context.swapchainImageIndex);
            image->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, false);
            return;
        }

        auto profiler = ServiceLocator::Get<IGpuProfiler>();
		auto statCollector = ServiceLocator::Get<IRenderStatCollector>();

        if (context.scene)
        {
            for (auto& pass : _passes) {
                if (pass->ShouldExecute(context))
                {
                    uint32_t measureIdx = profiler->StartPass(context.cmd, context.frameIndex, pass->GetGroup(), pass->GetName());

                    bool collectStats = pass->ShouldCollectStatistics();
                    uint32_t statIdx = 0;

                    if (collectStats) {
                        statIdx = statCollector->StartPass(context.cmd, context.frameIndex, pass->GetGroup(), pass->GetName());
                    }

                    pass->Execute(context);

                    if (collectStats) {
                        statCollector->EndPass(context.cmd, context.frameIndex, statIdx);
                    }

                    profiler->EndPass(context.cmd, context.frameIndex, measureIdx);
                }
            }
        }

        auto image = ServiceLocator::Get<Vk::Context>()->GetSwapChain()->GetImage(context.swapchainImageIndex);
		image->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, false);
    }
}