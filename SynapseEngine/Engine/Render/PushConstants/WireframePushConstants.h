#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <cstdint>

namespace Syn {
    struct alignas(16) WireframePushConstants {
        uint64_t animationAddressBuffer;
        uint64_t animationBufferAddr;
        uint64_t animationSparseMapBufferAddr;

        uint64_t modelAddressBuffer;
        uint64_t globalInstanceBuffers;
        uint64_t globalIndirectCommandDescriptorBuffers;

        uint64_t cameraBufferAddr;
        uint64_t cameraSparseMapBufferAddr;
        uint64_t transformBufferAddr;
        uint64_t transformSparseMapBufferAddr;

        uint64_t indexBufferAddr;
        uint64_t vertexBufferAddr;

        uint64_t debugInstanceBufferAddr;
        uint64_t modelBufferAddr;
        uint64_t modelSparseMapBufferAddr;

        uint32_t activeCameraEntity;
        uint32_t isSphere;
        uint32_t drawIdOffset;
        uint32_t padding;
        glm::vec4 debugColor;
    };
}