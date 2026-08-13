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
        Clearcoat = 7,
        ClearcoatRoughness = 8,
        ClearcoatNormal = 9,
        Specular = 10,
        SpecularColor = 11,
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