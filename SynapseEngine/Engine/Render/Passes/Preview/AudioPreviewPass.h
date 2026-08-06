#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"
#include <vector>

namespace Syn
{
    class SYN_API AudioPreviewPass : public TransferPass { 
    public:
        AudioPreviewPass() = default;

        std::string GetName() const override { return "AudioPreviewPass"; }
        std::string GetGroup() const override { return PassGroupNames::UtilityPasses; }

        void Initialize() override;
        bool ShouldCollectStatistics() const override { return false; }
    protected:
        void PrepareFrame(const RenderContext& context) override;
        void Transfer(const RenderContext& context) override;
    private:
        std::vector<uint32_t> _dirtyAudios;
    };
}