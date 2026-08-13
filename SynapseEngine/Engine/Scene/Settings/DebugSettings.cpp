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

#include "DebugSettings.h"

namespace Syn
{
    DebugSettings::DebugSettings()
        : useDebugCamera(false)
        , enableDebugVisibility(false)
        , debugVisibilityMode(DebugVisibilityMode::AllCombined)
        , enableWireframeMeshAabb(false)
        , enableWireframeMeshSphere(false)
        , enableWireframeMeshletAabb(false)
        , enableWireframeMeshletSphere(false)
        , enableWireframeMeshletCone(false)
        , enableStaticChunkAabbWireframe(false)
        , enableMortonChunkAabbWireframe(false)
        , enablePointLightSphereWireframe(false)
        , enablePointLightAabbWireframe(false)
        , enableSpotLightSphereWireframe(false)
        , enableSpotLightAabbWireframe(false)
        , enableSpotLightConeWireframe(false)
        , enableSpotLightPyramidWireframe(false)
        , enablePhysicsWireframe(false)
        , enableBillboardCameras(true)
        , enableBillboardPointLights(true)
        , enableBillboardSpotLights(true)
        , enableBillboardDirectionalLights(true)
        , enableSelectedOutline(true)
        , enableSelectedHierarchyOutline(true)
        , outlinePrimaryColor(glm::vec4(1.0f, 0.60f, 0.0f, 1.0f))
        , outlineSecondaryColor(glm::vec4(1.0f, 0.85f, 0.0f, 1.0f))
        , outlineThickness(2.0f)
        , enableInfiniteGrid(false)
        , gridScale(1.0f)
        , fadeDistance(250.0f)
        , gridThickness(1.0f)
        , axisThickness(2.0f)
        , gridShowXZ(true)
        , gridShowXY(false)
        , gridShowYZ(false)
        , gridShowAxisX(true)
        , gridShowAxisY(true)
        , gridShowAxisZ(true)
        , gridColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))
        , axisXColor(glm::vec4(1.0f, 0.2f, 0.2f, 1.0f))
        , axisYColor(glm::vec4(0.2f, 1.0f, 0.2f, 1.0f))
        , axisZColor(glm::vec4(0.2f, 0.2f, 1.0f, 1.0f))
    {}
}