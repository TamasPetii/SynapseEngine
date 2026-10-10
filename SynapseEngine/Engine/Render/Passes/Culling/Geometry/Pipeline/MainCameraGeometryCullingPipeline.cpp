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

#include "MainCameraGeometryCullingPipeline.h"

#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryCullingCommandResetPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryMortonChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryMortonModelCullingPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryStaticChunkCullingPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryStaticModelCullingPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryModelCullingPass.h"
#include "Engine/Render/Passes/Culling/Geometry/Passes/GeometryMeshCullingPass.h"

namespace Syn {
    MainCameraGeometryCullingPipeline::MainCameraGeometryCullingPipeline()
        : RenderPipeline("MainCameraGeometryCullingPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<GeometryCullingCommandResetPass>());
        AddPass(std::make_unique<GeometryMortonChunkCullingPass>());
        AddPass(std::make_unique<GeometryMortonModelCullingPass>());
        AddPass(std::make_unique<GeometryStaticChunkCullingPass>());
        AddPass(std::make_unique<GeometryStaticModelCullingPass>());
        AddPass(std::make_unique<GeometryModelCullingPass>());
        AddPass(std::make_unique<GeometryMeshCullingPass>());
    }
}