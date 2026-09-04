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
#include "Engine/Image/Source/Procedural/Pipeline/GpuComputePass.h"

namespace Syn
{
    class SYN_API BrdfLutComputePass : public GpuComputePass {
    public:
        BrdfLutComputePass(uint32_t sampleCount);
        void Initialize() override;
    protected:
        void BindDescriptors(const GpuProceduralContext& context) override;
        void PushConstants(const GpuProceduralContext& context) override;
        void Dispatch(const GpuProceduralContext& context) override;
    private:
        uint32_t _sampleCount;
    };
}