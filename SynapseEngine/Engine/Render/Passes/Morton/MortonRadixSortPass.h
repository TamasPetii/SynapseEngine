#pragma once
#include "Engine/SynApi.h"
#include "Engine/Render/Passes/TransferPass.h"

#include <vk_radix_sort.h>

namespace Syn {
    class SYN_API MortonRadixSortPass : public IRenderPass {
    public:
        ~MortonRadixSortPass();

        std::string GetName() const override { return "MortonRadixSortPass"; }
        std::string GetGroup() const override { return PassGroupNames::MortonPasses; }

        void Initialize() override;
    protected:
        bool ShouldExecute(const RenderContext& context) const override;
        void Execute(const RenderContext& context) override;
    private:
        uint32_t _staticCount = 0;
        VrdxSorter _radixSorter = VK_NULL_HANDLE;
        mutable bool _wasEnabled = false;
        mutable bool _needsRebuild = true;
        uint32_t _countdown = 0;
    };
}