#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {

    using GraphID = uint64_t;

    enum class GraphNodeType {
        Material,
        Texture
    };

    enum class GraphPinType {
        Albedo = 0,
        Normal = 1,
        Metalness = 2,
        Roughness = 3,
        MetallicRoughness = 4,
        Emissive = 5,
        AmbientOcclusion = 6,
        TextureOutput = 99
    };

    struct GraphPinData {
        GraphID id;
        GraphID parentNodeId;
        GraphPinType type;
        bool isInput;
    };

    struct GraphNodeData {
        GraphID id;
        GraphNodeType type;
        uint32_t engineResourceId;
        std::string name;
        std::vector<GraphPinData> pins;
        TextureHandle textureHandle = InvalidTextureHandle;
    };

    struct GraphLinkData {
        GraphID id;
        GraphID startPinId;
        GraphID endPinId;
    };

    struct MaterialGraphState {
        bool isMaterialSelected = false;
        uint32_t selectedMaterialId = 0xFFFFFFFF;
        std::string selectedMaterialName = "";

        std::vector<GraphNodeData> nodes;
        std::vector<GraphLinkData> links;
        GraphID nextId = 1;
    };
}