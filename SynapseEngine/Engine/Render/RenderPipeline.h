#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <memory>

#include "IRenderPass.h"

namespace Syn
{
    class SYN_API RenderPipeline
    {
    public:
        RenderPipeline() = default;

        RenderPipeline(const RenderPipeline&) = delete;
        RenderPipeline& operator=(const RenderPipeline&) = delete;

        void AddPass(std::unique_ptr<IRenderPass> pass);
        void InitializeAll();
        void Execute(const RenderContext& context);
    private:
        std::vector<std::unique_ptr<IRenderPass>> _passes;
    };
}