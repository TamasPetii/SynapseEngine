#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* TraditionalVert = "../Engine/Shaders/Traditional.vert";
        static constexpr const char* TraditionalFrag = "../Engine/Shaders/Traditional.frag";
        static constexpr const char* MeshletMesh = "../Engine/Shaders/Meshlet.mesh";
        static constexpr const char* MeshletFrag = "../Engine/Shaders/Meshlet.frag";
    };
}