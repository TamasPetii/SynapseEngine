#pragma once
#include "Engine/Render/IRenderPass.h"

namespace Syn 
{
    class SYN_API ClusterLightWriteSyncPassDBC : public IRenderPass {
    public:
        void Execute(const RenderContext& context) override;
        std::string GetName() const override { return "ClusterLightWriteSyncPassDBC"; }
        std::string GetGroup() const override { return PassGroupNames::ForwardPlusClusterPasses; }
    };
}