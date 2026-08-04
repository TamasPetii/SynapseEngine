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