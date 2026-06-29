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