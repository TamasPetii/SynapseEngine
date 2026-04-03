#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Common/VertexSkinData.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include "Engine/Mesh/Data/Gpu/GpuMeshletDrawData.h"
#include "Engine/Mesh/Data/Gpu/GpuNodeTransform.h"
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API GpuAnimationDescriptor
    {
        uint32_t frameCount;
        uint32_t nodeCount;
        uint32_t globalVertexCount;
        uint32_t globalMeshCount;
        uint32_t globalMeshletCount;

        float durationInSeconds;
        float sampleRate;
        float padding;
    };

    struct SYN_API GpuBatchedAnimation
    {
        GpuAnimationDescriptor descriptor;

        // Flattened by meshNodeDescriptors!
        std::vector<VertexSkinData> vertexSkinData;

        // [Frame0_Node0... Frame0_NodeN | Frame1_Node0... Frame1_NodeN]
        std::vector<GpuNodeTransform> nodeTransforms;

        // [Frame0, Frame1, Frame2...]
        std::vector<GpuMeshCollider> frameGlobalColliders;

        // [Frame0_Instance0... Frame0_InstanceN | Frame1_Instance0... ]
        std::vector<GpuMeshCollider> frameMeshColliders;

        // [Frame0_Meshlet0... Frame0_MeshletN | Frame1_Meshlet0... ]
        std::vector<GpuMeshletCollider> frameMeshletColliders;
    };
}