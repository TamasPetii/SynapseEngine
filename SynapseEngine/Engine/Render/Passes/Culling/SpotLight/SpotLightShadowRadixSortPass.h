#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

#include <vk_radix_sort.h>

namespace Syn {
    class SYN_API SpotLightShadowRadixSortPass : public IRenderPass {
    public:
        ~SpotLightShadowRadixSortPass();

        std::string GetName() const override { return "SpotLightShadowRadixSortPass"; }
        std::string GetGroup() const override { return PassGroupNames::SpotLightCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void Execute(const RenderContext& context) override;
    private:
        VrdxSorter _radixSorter = VK_NULL_HANDLE;
    };
}