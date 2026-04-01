#include "RenderPipeline.h"

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
        if (!context.scene) return;

        for (auto& pass : _passes) {
            if (pass->ShouldExecute(context))
            {
                pass->Execute(context);
            }
        }
    }
}