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
#include "Engine/Scene/Settings/SceneSettings.h"
#include "ModelDrawGroup.h"
#include "DebugDrawGroup.h"
#include "PointLightDrawGroup.h"
#include "SpotLightDrawGroup.h"
#include "DirectionLightDrawGroup.h"
#include "ForwardPlusDrawGroup.h"
#include "ChunkDrawGroup.h"
#include "SsaoDrawGroup.h"
#include <atomic>
#include "IDrawGroup.h"
#include "DirectionLightShadowDrawGroup.h"
#include "SpotLightShadowDrawGroup.h"
#include "PointLightShadowDrawGroup.h"

namespace Syn
{
    class SYN_API SceneDrawData : public IDrawGroup
    {
    public:
        SceneDrawData(uint32_t frameCount);

        void RequestGlobalSync(uint32_t framesInFlight);
		virtual void CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) override;

        RenderBuffer frameContextBuffer;
        ModelDrawGroup Models;
        DebugDrawGroup Debug;
        PointLightDrawGroup PointLights;
        ForwardPlusDrawGroup ForwardPlus;
		ChunkDrawGroup Chunks;
		SsaoDrawGroup Ssao;
        DirectionLightDrawGroup DirectionLights;
        DirectionLightShadowDrawGroup DirectionLightShadow;
        SpotLightDrawGroup SpotLights;
        SpotLightShadowDrawGroup SpotLightShadow;
        PointLightShadowDrawGroup PointLightShadow;

        std::atomic<uint32_t> syncFramesRemaining{ 0 };
    };
}