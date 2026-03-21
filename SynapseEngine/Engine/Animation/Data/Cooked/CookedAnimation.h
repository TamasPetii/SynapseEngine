#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimationFrames.h"
#include "Engine/Animation/Data/Common/BoneTrack.h"
#include "Engine/Animation/Data/Common/AnimationNode.h"
#include <string>
#include <vector>

namespace Syn
{
    struct SYN_API CookedAnimationMeshSkin
    {
        std::vector<VertexSkinData> vertices;
    };

    struct SYN_API CookedAnimation
    {
        std::string name;
        float durationInSeconds;
        float ticksPerSecond;
        float sampleRate;
        uint32_t frameCount;
        uint32_t nodeCount;

        std::vector<CookedAnimationMeshSkin> meshSkins;
        std::vector<BoneTrack> tracks;
        std::vector<AnimationNode> nodes;
        std::vector<CookedAnimationFrame> frames;
    };
}