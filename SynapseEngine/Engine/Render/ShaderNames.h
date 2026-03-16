#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API ShaderNames
    {
        static constexpr const char* TraditionalVert = "../Engine/Shaders/Traditional.vert";
        static constexpr const char* TraditionalFrag = "../Engine/Shaders/Traditional.frag";

        static constexpr const char* MeshletOldMesh = "../Engine/Shaders/MeshletOld.mesh";
        static constexpr const char* MeshletMesh = "../Engine/Shaders/Meshlet.mesh";
        static constexpr const char* MeshletTask = "../Engine/Shaders/Meshlet.task";
        static constexpr const char* MeshletFrag = "../Engine/Shaders/Meshlet.frag";

        static constexpr const char* MeshCulling = "../Engine/Shaders/MeshCulling.comp";
        static constexpr const char* ModelCulling = "../Engine/Shaders/ModelCulling.comp";
    };
}