#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    struct SYN_API MeshDrawDescriptor
    {
        uint32_t modelIndex;        // Model ID for vertex/index/material buffer lookups
        uint32_t meshIndex;         // Submesh index
        uint32_t lodIndex;          // Level of Detail index
        uint32_t instanceOffset;    // Start offset in the global instance buffer
        uint32_t maxInstances;      // Window capacity (prevents overflow)
        uint32_t indirectIndex;     // Index in the global indirect draw command buffer
        uint32_t isMeshletPipeline; // 0 = Traditional, 1 = Mesh Shader
        uint32_t padding;           // 16-byte std430 alignment
    };
}