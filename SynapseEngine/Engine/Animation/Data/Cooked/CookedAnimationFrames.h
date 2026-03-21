#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Common/VertexSkinData.h"
#include "Engine/Mesh/Data/Cooked/CookedColliders.h"
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API CookedAnimationFrameMeshlet
    {
        CookedMeshletCollisionData collider;
    };

    struct SYN_API CookedAnimationFrameLod
    {
        std::vector<CookedAnimationFrameMeshlet> meshlets;
    };

    struct SYN_API CookedAnimationFrameMesh
    {
        CookedMeshCollisionData collider;
        std::vector<CookedAnimationFrameLod> lods;
    };

    struct SYN_API CookedAnimationFrame
    {
        CookedMeshCollisionData globalCollider;
        std::vector<CookedAnimationFrameMesh> meshes;
        std::vector<glm::mat4> bakedNodeTransforms;
    };
}