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
#include <string>
#include <glm/glm.hpp>

namespace Syn {
    struct ModelPropertiesState {
        bool hasSelection = false;
        uint32_t selectedModelId = 0xFFFFFFFF;
        int32_t selectedDescriptorIndex = -1;
        std::string modelName = "";

        uint32_t globalVertexCount = 0;
        uint32_t globalIndexCount = 0;
        uint32_t globalMeshCount = 0;
        glm::vec3 globalAabbMin = glm::vec3(0.0f);
        glm::vec3 globalAabbMax = glm::vec3(0.0f);
        glm::vec3 globalCenter = glm::vec3(0.0f);
        float globalRadius = 0.0f;

        bool isNodeSelected = false;
        std::string nodeName = "";
        uint32_t nodeVertexCount = 0;
        uint32_t nodeIndexCount = 0;
        int32_t meshIndex = -1;
        int32_t nodeIndex = -1;
        int32_t parentNodeIndex = -1;

        uint32_t nodeMeshletCount = 0;
        uint32_t nodeMaterialIndex = 0;
        glm::vec3 nodeAabbMin = glm::vec3(0.0f);
        glm::vec3 nodeAabbMax = glm::vec3(0.0f);
        float nodeRadius = 0.0f;
    };
}