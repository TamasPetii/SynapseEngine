#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <memory>

#include "IRenderPass.h"
#include "Data/RenderScene.h"

namespace Syn
{
    class SYN_API RenderPipeline
    {
    public:
        void AddPass(std::unique_ptr<IRenderPass> pass);
        void InitializeAll(std::shared_ptr<ResourceManager> resourceManager);
        void Execute(VkCommandBuffer cmd, const RenderScene& scene);
    private:
        std::vector<std::unique_ptr<IRenderPass>> _passes;
    };
}