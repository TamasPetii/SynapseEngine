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