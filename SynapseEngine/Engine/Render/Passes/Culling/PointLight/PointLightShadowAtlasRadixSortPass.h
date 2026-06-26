#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/IRenderPass.h"

#include <vk_radix_sort.h>

namespace Syn {
    class SYN_API PointLightShadowAtlasRadixSortPass : public IRenderPass {
    public:
        ~PointLightShadowAtlasRadixSortPass();

        std::string GetName() const override { return "PointLightShadowAtlasRadixSortPass"; }
        std::string GetGroup() const override { return PassGroupNames::PointLightCullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void Execute(const RenderContext& context) override;
    private:
        VrdxSorter _radixSorter = VK_NULL_HANDLE;
    };
}