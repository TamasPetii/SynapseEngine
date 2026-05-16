#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/ComputePass.h"

#include <vk_radix_sort.h>

namespace Syn {
    class SYN_API MortonRadixSortPass : public ComputePass {
    public:
        ~MortonRadixSortPass();

        std::string GetName() const override { return "MortonRadixSortPass"; }
        std::string GetGroup() const override { return PassGroupNames::CullingPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void Dispatch(const RenderContext& context) override;
    private:
        uint32_t _staticCount = 0;
        VrdxSorter _radixSorter = VK_NULL_HANDLE;
    };
}