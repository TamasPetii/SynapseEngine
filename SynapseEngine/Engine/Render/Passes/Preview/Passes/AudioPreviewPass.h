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