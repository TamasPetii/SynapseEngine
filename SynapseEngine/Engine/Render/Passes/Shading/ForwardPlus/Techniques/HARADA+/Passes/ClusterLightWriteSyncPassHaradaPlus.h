#pragma once
#include "Engine/Render/IRenderPass.h"

namespace Syn {
    class SYN_API ClusterLightWriteSyncPassHaradaPlus : public IRenderPass {
    public:
        void Execute(const RenderContext& context) override;
        std::string GetName() const override { return "ClusterLightWriteSyncPassHaradaPlus"; }
        std::string GetGroup() const override { return PassGroupNames::ForwardPlusClusterPasses; }
    };
}