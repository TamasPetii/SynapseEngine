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

#include "PhysicsDebugSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "Engine/System/SystemPhaseNames.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Physics/BoxColliderSystem.h"
#include "Engine/System/Physics/SphereColliderSystem.h"
#include "Engine/System/Physics/CapsuleColliderSystem.h"
#include "Engine/System/Physics/ConvexColliderSystem.h"
#include "Engine/System/Physics/MeshColliderSystem.h"

namespace Syn
{
    std::vector<TypeID> PhysicsDebugSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<RigidBodySystem>::ID,
            TypeInfo<BoxColliderSystem>::ID,
            TypeInfo<SphereColliderSystem>::ID,
            TypeInfo<CapsuleColliderSystem>::ID,
            TypeInfo<ConvexColliderSystem>::ID,
            TypeInfo<MeshColliderSystem>::ID
        };
    }

    std::vector<TypeID> PhysicsDebugSystem::GetWriteDependencies() const
    {
        return { TypeInfo<PhysicsDebugSystem>::ID };
    }

    void PhysicsDebugSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto physicsEngine = scene->GetPhysicsEngine();
        if (!physicsEngine) return;

        if (!scene->GetSettings()->debug.enablePhysicsWireframe)
            return;

        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, physicsEngine, frameIndex]()
            {
                PhysicsDrawData drawData;
                physicsEngine->GetDebugDrawData(drawData);

                if (drawData.indirectCmds.empty())
                {
                    auto sceneDrawData = scene->GetSceneDrawData();
                    if (sceneDrawData) sceneDrawData->Debug.joltDebugIndirectCount = 0;
                    return;
                }

                auto sceneDrawData = scene->GetSceneDrawData();
                if (!sceneDrawData) return;

                sceneDrawData->Debug.joltDebugVertexBuffer.UpdateCapacity(frameIndex, drawData.vertices.size());
                sceneDrawData->Debug.joltDebugVertexBuffer.Write(
                    frameIndex,
                    drawData.vertices.data(),
                    drawData.vertices.size() * sizeof(PhysicsDebugVertex),
                    0
                );

                sceneDrawData->Debug.joltDebugIndexBuffer.UpdateCapacity(frameIndex, drawData.indices.size());
                sceneDrawData->Debug.joltDebugIndexBuffer.Write(
                    frameIndex,
                    drawData.indices.data(),
                    drawData.indices.size() * sizeof(uint32_t),
                    0
                );

                sceneDrawData->Debug.joltDebugInstanceBuffer.UpdateCapacity(frameIndex, drawData.instances.size());
                sceneDrawData->Debug.joltDebugInstanceBuffer.Write(
                    frameIndex,
                    drawData.instances.data(),
                    drawData.instances.size() * sizeof(PhysicsDebugInstance),
                    0
                );

                sceneDrawData->Debug.joltDebugIndirectCount = drawData.indirectCmds.size();
                sceneDrawData->Debug.joltDebugIndirectBuffer.UpdateCapacity(frameIndex, drawData.indirectCmds.size());
                sceneDrawData->Debug.joltDebugIndirectBuffer.Write(
                    frameIndex,
                    drawData.indirectCmds.data(),
                    drawData.indirectCmds.size() * sizeof(VkDrawIndirectCommand),
                    0
                );
            });
    }
}