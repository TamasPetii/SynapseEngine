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
#include <glm/glm.hpp>

namespace Syn
{
    enum SYN_API DebugVisibilityMode
    {
        EntityId = 0,
        Pipeline = 1,
        LodLevel = 2,
        MeshIndex = 3,
        MeshletIndex = 4,
        TriangleIndex = 5,
        AllCombined = 6,
        MaterialType = 7,
        ForwardPlusSlices = 8,
        ForwardPlusLightCount = 9
    };

    struct SYN_API DebugSettings
    {
        DebugSettings();

        bool useDebugCamera;
        bool enableDebugVisibility;
        DebugVisibilityMode debugVisibilityMode;

        // Geometry Wireframes
        bool enableWireframeMeshAabb;
        bool enableWireframeMeshSphere;
        bool enableWireframeMeshletAabb;
        bool enableWireframeMeshletSphere;
        bool enableWireframeMeshletCone;
        bool enableStaticChunkAabbWireframe;
        bool enableMortonChunkAabbWireframe;

        // Light Wireframes
        bool enablePointLightSphereWireframe;
        bool enablePointLightAabbWireframe;
        bool enableSpotLightSphereWireframe;
        bool enableSpotLightAabbWireframe;
        bool enableSpotLightConeWireframe;
        bool enableSpotLightPyramidWireframe;

        // Physics Colliders
        bool enablePhysicsWireframe;

        // Editor Billboards
        bool enableBillboardCameras;
        bool enableBillboardPointLights;
        bool enableBillboardSpotLights;
        bool enableBillboardDirectionalLights;

        // Editor Selection Outlines
        bool enableSelectedOutline;
        bool enableSelectedHierarchyOutline;
        glm::vec4 outlinePrimaryColor;
        glm::vec4 outlineSecondaryColor;
        float outlineThickness;

        // Infinite grid
        bool enableInfiniteGrid;
        float gridScale;
        float fadeDistance;
        float gridThickness;
        float axisThickness;
        bool gridShowXZ;
        bool gridShowXY;
        bool gridShowYZ;
        bool gridShowAxisX;
        bool gridShowAxisY;
        bool gridShowAxisZ;
        glm::vec4 gridColor;
        glm::vec4 axisXColor;
        glm::vec4 axisYColor;
        glm::vec4 axisZColor;
    };
}