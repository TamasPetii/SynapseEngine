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
#include "Engine/Material/MaterialManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Audio/AudioManager.h"
#include <vector>

namespace Syn {
    struct SYN_API SystemContext {
        float deltaTime;
        uint32_t frameIndex;

        uint32_t modelManagerVersion;
        uint32_t materialManagerVersion;
        uint32_t animationManagerVersion;
		uint32_t audioManagerVersion;

        std::vector<MaterialRenderType> materialRenderTypes;
        std::vector<BaseResourceManager<StaticMesh>::ResourceSnapshot> modelSnapshots;
        std::vector<BaseResourceManager<Material>::ResourceSnapshot> materialSnapshots;
        std::vector<BaseResourceManager<Animation>::ResourceSnapshot> animationSnapshots;
        std::vector<BaseResourceManager<Sound>::ResourceSnapshot> soundSnapshots;
    };
}