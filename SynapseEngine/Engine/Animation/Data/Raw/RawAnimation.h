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
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Engine/Animation/Data/Common/VertexSkinData.h"
#include "Engine/Animation/Data/Common/BoneTrack.h"
#include "Engine/Animation/Data/Common/AnimationNode.h"

namespace Syn
{
    struct SYN_API RawMeshSkin
    {
        std::vector<VertexSkinData> vertices;
    };

    struct SYN_API RawAnimation
    {
        std::string name;
        float duration = 0.0f;
        float ticksPerSecond = 0.0f;

        std::vector<RawMeshSkin> meshSkins;
        std::vector<BoneTrack> tracks;
        std::vector<AnimationNode> nodes; //Flat BFS (Parent -> Child)
    };
}