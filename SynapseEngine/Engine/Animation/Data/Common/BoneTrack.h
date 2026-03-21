#pragma once
#include "Engine/SynApi.h"
#include "AnimationKeys.h"
#include <string>
#include <vector>

namespace Syn
{
    struct SYN_API BoneTrack
    {
        std::string nodeName;
        std::vector<AnimationKeyPosition> positions;
        std::vector<AnimationKeyRotation> rotations;
        std::vector<AnimationKeyScale> scales;
    };
}