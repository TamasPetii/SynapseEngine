#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/GraphicsPass.h"

namespace Syn {
    class SYN_API OpaqueInitPass : public GraphicsPass {
    public:
        std::string GetName() const override { return "OpaqueInitPass"; }
        std::string GetGroup() const override { return PassGroupNames::ShadingSetupPasses; }

        void Execute(const RenderContext& context) override;
    };
}