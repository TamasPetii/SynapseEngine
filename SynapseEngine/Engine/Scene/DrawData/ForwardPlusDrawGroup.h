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
#include "IDrawGroup.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn
{
    struct SYN_API ForwardPlusDispatchCmd {
        uint32_t x, y, z, pad;
    };

    struct SYN_API ForwardPlusDispatchArgs {
        ForwardPlusDispatchCmd pointFastPath;
        ForwardPlusDispatchCmd pointSlowCount;
        ForwardPlusDispatchCmd pointSlowWrite;

        ForwardPlusDispatchCmd spotFastPath;
        ForwardPlusDispatchCmd spotSlowCount;
        ForwardPlusDispatchCmd spotSlowWrite;

        ForwardPlusDispatchCmd prefixSum;
    };

    struct SYN_API ForwardPlusDrawGroup : public IDrawGroup 
    {
        ForwardPlusDrawGroup(uint32_t frameCount);	
        virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;
        void CheckResize(uint32_t tileSize,uint32_t width, uint32_t height, uint32_t frameIndex);

        RenderBuffer tileGridBuffer;
        RenderBuffer clusterListBuffer;
        RenderBuffer clusterCountBuffer;

        RenderBuffer dispatchArgsBuffer;

        RenderBuffer pointLightIndexBuffer;
        RenderBuffer spotLightIndexBuffer;

        uint32_t maxClusters = 0;
    };
}