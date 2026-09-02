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

#include "WireframePipeline.h"
#include "Engine/Render/Passes/Wireframe/Mesh/WireframeMeshSetupPass.h"
#include "Engine/Render/Passes/Wireframe/Mesh/WireframeMeshAabbPass.h"
#include "Engine/Render/Passes/Wireframe/Mesh/WireframeMeshSpherePass.h"
#include "Engine/Render/Passes/Wireframe/Light/PointLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Light/PointLightSphereWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Light/SpotLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Light/SpotLightSphereWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Light/SpotLightConeWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Light/SpotLightPyramidWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Chunk/StaticChunkAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Chunk/MortonChunkAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/Physics/WireframePhysicsPass.h"
#include "Engine/Render/Passes/Wireframe/Meshlet/WireframeMeshletAabbPass.h"
#include "Engine/Render/Passes/Wireframe/Meshlet/WireframeMeshletSpherePass.h"
#include "Engine/Render/Passes/Wireframe/Meshlet/WireframeMeshletConePass.h"

namespace Syn {
    WireframePipeline::WireframePipeline()
        : RenderPipeline("WireframePipeline", PassGroupNames::DebugPasses)
    {
        AddPass(std::make_unique<WireframeMeshSetupPass>());
        AddPass(std::make_unique<WireframeMeshAabbPass>());
        AddPass(std::make_unique<WireframeMeshSpherePass>());
        AddPass(std::make_unique<WireframeMeshletAabbPass>());
        AddPass(std::make_unique<WireframeMeshletSpherePass>());
        AddPass(std::make_unique<WireframeMeshletConePass>());
        AddPass(std::make_unique<PointLightAabbWireframePass>());
        AddPass(std::make_unique<PointLightSphereWireframePass>());
        AddPass(std::make_unique<SpotLightAabbWireframePass>());
        AddPass(std::make_unique<SpotLightSphereWireframePass>());
        AddPass(std::make_unique<SpotLightConeWireframePass>());
        AddPass(std::make_unique<SpotLightPyramidWireframePass>());
        AddPass(std::make_unique<StaticChunkAabbWireframePass>());
        AddPass(std::make_unique<MortonChunkAabbWireframePass>());
        AddPass(std::make_unique<WireframePhysicsPass>());
    }

    bool WireframePipeline::ShouldExecute(const RenderContext& context) const {
        return true;
    }
}