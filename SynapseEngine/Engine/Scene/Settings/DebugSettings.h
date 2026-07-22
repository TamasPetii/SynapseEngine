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