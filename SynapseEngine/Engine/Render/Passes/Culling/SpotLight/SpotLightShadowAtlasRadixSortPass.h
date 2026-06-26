#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/IRenderPass.h"

#include <vk_radix_sort.h>

namespace Syn {
    class SYN_API SpotLightShadowAtlasRadixSortPass : public IRenderPass {
    public:
        ~SpotLightShadowAtlasRadixSortPass();

        std::string GetName() const override { return "SpotLightShadowAtlasRadixSortPass"; }
        std::string GetGroup() const override { return PassGroupNames::SpotLightCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void Execute(const RenderContext& context) override;
    private:
        VrdxSorter _radixSorter = VK_NULL_HANDLE;
    };
}