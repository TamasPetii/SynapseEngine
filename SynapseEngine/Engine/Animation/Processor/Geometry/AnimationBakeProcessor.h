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
#include "Engine/Animation/Processor/IAnimationProcessor.h"

namespace Syn
{
    class SYN_API AnimationBakeProcessor : public IAnimationProcessor
    {
    public:
        virtual void Process(CookedAnimation& inOutAnimation, const CpuModelData& baseModel) override;
    private:
        void BakeFrame(uint32_t frameIndex, CookedAnimation& anim);

        glm::vec3 InterpolatePosition(const BoneTrack& track, float time);
        glm::quat InterpolateRotation(const BoneTrack& track, float time);
        glm::vec3 InterpolateScale(const BoneTrack& track, float time);
    };
}