// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
        mutable uint32_t _lastMortonVersion = 0;
    };
}