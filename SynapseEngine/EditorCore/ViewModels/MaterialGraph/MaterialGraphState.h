#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace Syn {

    using GraphID = uint64_t;

    enum class GraphNodeType { 
        Material,
        Texture
    };

    enum class GraphPinType {
        Albedo, 
        Normal, 
        Metalness,
        Roughness,
        MetallicRoughness,
        Emissive,
        AmbientOcclusion,
        TextureOutput
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
    };

    struct GraphLinkData {
        GraphID id;
        GraphID startPinId;
        GraphID endPinId;
    };

    struct MaterialGraphState {
        std::vector<GraphNodeData> nodes;
        std::vector<GraphLinkData> links;
        GraphID nextId = 1;
    };
}