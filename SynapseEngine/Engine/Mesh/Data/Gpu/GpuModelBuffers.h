// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include "Engine/Mesh/MeshDrawBlueprint.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"

namespace Syn
{
    struct SYN_API GpuModelBuffers
    {
        // Geometry Buffers
        std::unique_ptr<Vk::Buffer> vertexPositions;
        std::unique_ptr<Vk::Buffer> vertexAttributes;
        std::unique_ptr<Vk::Buffer> indices;
        std::unique_ptr<Vk::Buffer> meshMaterialIndices;

        // Traditional Pipeline Buffers
        std::unique_ptr<Vk::Buffer> meshDescriptors;
        std::unique_ptr<Vk::Buffer> meshColliders;
        std::unique_ptr<Vk::Buffer> lodDescriptors;

        // Mesh Shader Buffers
        std::unique_ptr<Vk::Buffer> meshletVertexIndices;
        std::unique_ptr<Vk::Buffer> meshletTriangleIndices;
        std::unique_ptr<Vk::Buffer> meshletDescriptors;
        std::unique_ptr<Vk::Buffer> meshletDrawDescriptors;
        std::unique_ptr<Vk::Buffer> meshletColliders;

        //Hierarchy Buffers
        std::unique_ptr<Vk::Buffer> nodeTransforms;
    };

    struct SYN_API GpuModelAddresses
    {
        VkDeviceAddress vertexPositions;
        VkDeviceAddress vertexAttributes;
        VkDeviceAddress indices;
        VkDeviceAddress meshMaterialIndices;
        VkDeviceAddress meshDescriptors;
        VkDeviceAddress meshColliders;
        VkDeviceAddress lodDescriptors;
        VkDeviceAddress meshletVertexIndices;
        VkDeviceAddress meshletTriangleIndices;
        VkDeviceAddress meshletDescriptors;
        VkDeviceAddress meshletDrawDescriptors;
        VkDeviceAddress meshletColliders;
        VkDeviceAddress nodeTransforms;

        uint32_t isReady;
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t averageLodIndexCount;
        uint32_t meshCount;
        uint32_t padding0;

        GpuMeshCollider globalCollider;
    };
}