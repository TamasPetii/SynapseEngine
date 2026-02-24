#include "RenderPipeline.h"

namespace Syn
{
    void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
    {
        _passes.push_back(std::move(pass));
    }

    void RenderPipeline::InitializeAll(std::shared_ptr<ResourceManager> resourceManager)
    {
        for (auto& pass : _passes) {
            pass->Initialize(resourceManager);
        }
    }

    void RenderPipeline::Execute(VkCommandBuffer cmd, const RenderScene& scene)
    {
        for (auto& pass : _passes) {
            pass->Execute(cmd, scene);
        }
    }
}