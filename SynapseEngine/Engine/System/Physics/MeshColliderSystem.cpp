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

#include "MeshColliderSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "PhysicsUtils.h"

namespace Syn
{
    std::vector<TypeID> MeshColliderSystem::GetReadDependencies() const
    {
        return { 
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID
        };
    }

    std::vector<TypeID> MeshColliderSystem::GetWriteDependencies() const
    {
        return { TypeInfo<RigidBodySystem>::ID, TypeInfo<MeshColliderSystem>::ID };
    }

    void MeshColliderSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto meshColliderPool = registry->GetPool<MeshColliderComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto physicsEngine = scene->GetPhysicsEngine();

        if (!meshColliderPool || !rbPool || !transformPool || !modelPool || !physicsEngine) return;

        auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;

        ParallelForEach(meshColliderPool, subflow, SystemPhaseNames::Update, [meshColliderPool, rbPool, transformPool, modelPool, physicsEngine, modelSnapshots](EntityID entity) {

            if (!rbPool->Has(entity) || !transformPool->Has(entity) || !modelPool->Has(entity)) return;

            auto& meshColl = meshColliderPool->Get(entity);
            auto& rb = rbPool->Get(entity);
            auto& tr = transformPool->Get(entity);
            auto& modelComp = modelPool->Get(entity);

            uint32_t modelIndex = modelComp.modelIndex;
            if (modelIndex >= modelSnapshots.size()) return;

            auto model = modelSnapshots[modelIndex].resource;
            if (!model || model->cpuData.vertices.empty() || model->cpuData.indices.empty()) return;

            uint32_t lod = glm::min(meshColl.targetLodLevel, 3u);
            if (lod >= model->cpuData.physicsIndicesPerLod.size() || model->cpuData.physicsIndicesPerLod[lod].empty()) return;

            const auto& vertices = model->cpuData.physicsVertices;
            const auto& indices = model->cpuData.physicsIndicesPerLod[lod];

            if (rb.bodyID == INVALID_BODY_ID)
            {
                rb.bodyID = PhysicsUtils::TryCreateBody(entity, &tr, rb,
                    [&](const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const PhysicsBodySettings& settings) {
                        return physicsEngine->CreateMeshBody(pos, rot, vertices, indices, scale, settings);
                    });
            }
            else if (meshColliderPool->IsBitSet<UPDATE_BIT>(entity))
            {
                glm::vec3 worldScale = PhysicsUtils::ExtractScale(tr.transform);
                physicsEngine->SetMeshShape(rb.bodyID, vertices, indices, worldScale);
            }

            meshColl.version++;
            });
    }

    void MeshColliderSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto meshColliderPool = registry->GetPool<MeshColliderComponent>();
        if (!meshColliderPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::MeshColliderData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<MeshColliderComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [meshColliderPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& meshColl = meshColliderPool->Get(entity);
            auto index = meshColliderPool->GetMapping().Get(entity);

            if (componentBuffer.versions[index] != meshColl.version)
            {
                componentBuffer.versions[index] = meshColl.version;
                bufferHandler[index] = MeshColliderComponentGPU(meshColl);
            }
            };

        ForEachStream(meshColliderPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(meshColliderPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(meshColliderPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}