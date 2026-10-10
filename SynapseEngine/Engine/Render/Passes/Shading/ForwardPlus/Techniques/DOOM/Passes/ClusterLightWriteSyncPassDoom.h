#pragma once
#include "Engine/Render/IRenderPass.h"

namespace Syn 
{
    class SYN_API ClusterLightWriteSyncPassDoom : public IRenderPass {
    public:
        void Execute(const RenderContext& context) override;
        std::string GetName() const override { return "ClusterLightWriteSyncPassDoom"; }
    };
}