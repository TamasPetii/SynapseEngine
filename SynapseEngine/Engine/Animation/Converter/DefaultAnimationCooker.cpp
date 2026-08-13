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

#include "DefaultAnimationCooker.h"
#include "Engine/Mesh/Data/Common/TransformNode.h"
#include <cmath>

namespace Syn
{
    CookedAnimation DefaultAnimationCooker::Cook(const RawAnimation& rawAnimation, float sampleRate)
    {
        CookedAnimation cookedAnim;

        cookedAnim.name = rawAnimation.name;
        cookedAnim.ticksPerSecond = rawAnimation.ticksPerSecond > 0.0f ? rawAnimation.ticksPerSecond : 60.0f;
        cookedAnim.durationInSeconds = rawAnimation.duration / cookedAnim.ticksPerSecond;
        cookedAnim.sampleRate = sampleRate;

        cookedAnim.frameCount = static_cast<uint32_t>(std::ceil(cookedAnim.durationInSeconds * sampleRate)) + 1;
        cookedAnim.nodeCount = static_cast<uint32_t>(rawAnimation.nodes.size());

        cookedAnim.tracks = rawAnimation.tracks;
        cookedAnim.nodes = rawAnimation.nodes;

        cookedAnim.meshSkins.reserve(rawAnimation.meshSkins.size());

        for (const auto& rawSkin : rawAnimation.meshSkins)
        {
            CookedAnimationMeshSkin cookedSkin;
            cookedSkin.vertices = rawSkin.vertices;
            cookedAnim.meshSkins.push_back(std::move(cookedSkin));
        }

        cookedAnim.frames.resize(cookedAnim.frameCount);
        for (auto& frame : cookedAnim.frames)
        {
            frame.bakedNodeTransforms.resize(cookedAnim.nodeCount);
        }

        return cookedAnim;
    }
}