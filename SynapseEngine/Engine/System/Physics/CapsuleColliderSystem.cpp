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

#include "CapsuleColliderSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "PhysicsUtils.h"

#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Core/TransformSystem.h"

namespace Syn
{
    std::vector<TypeID> CapsuleColliderSystem::GetReadDependencies() const { 
        return { 
            TypeInfo<TransformSystem>::ID 
        };
    }

    std::vector<TypeID> CapsuleColliderSystem::GetWriteDependencies() const { 
        return { 
            TypeInfo<RigidBodySystem>::ID,
            TypeInfo<CapsuleColliderSystem>::ID };
    }

    void CapsuleColliderSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto capsulePool = registry->GetPool<CapsuleColliderComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto physicsEngine = scene->GetPhysicsEngine();

        if (!capsulePool || !rbPool || !transformPool || !physicsEngine) return;

        ParallelForEach(capsulePool, subflow, SystemPhaseNames::Update, [capsulePool, rbPool, transformPool, physicsEngine](EntityID entity) {
            if (!rbPool->Has(entity) || !transformPool->Has(entity)) return;

            auto& capsule = capsulePool->Get(entity);
            auto& rb = rbPool->Get(entity);
            auto& tr = transformPool->Get(entity);

            if (rb.bodyID == INVALID_BODY_ID)
            {
                rb.bodyID = PhysicsUtils::TryCreateBody(entity, &tr, rb,
                    [&](const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const PhysicsBodySettings& settings) {
                        float maxRadScale = glm::max(scale.x, scale.z);
                        return physicsEngine->CreateCapsuleBody(pos, rot, capsule.halfHeight * scale.y, capsule.radius * maxRadScale, settings);
                    });
            }
            else if (capsulePool->IsBitSet<UPDATE_BIT>(entity))
            {
                glm::vec3 worldScale = PhysicsUtils::ExtractScale(tr.transform);
                float maxRadScale = glm::max(worldScale.x, worldScale.z);
                physicsEngine->SetCapsuleShape(rb.bodyID, capsule.halfHeight * worldScale.y, capsule.radius * maxRadScale);
            }

            capsule.version++;
            });
    }

    void CapsuleColliderSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto capsulePool = registry->GetPool<CapsuleColliderComponent>();
        if (!capsulePool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::CapsuleColliderData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<CapsuleColliderComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [capsulePool, bufferHandler, componentBuffer](EntityID entity) {
            auto& capsule = capsulePool->Get(entity);
            auto capsuleIndex = capsulePool->GetMapping().Get(entity);

            if (componentBuffer.versions[capsuleIndex] != capsule.version)
            {
                componentBuffer.versions[capsuleIndex] = capsule.version;
                bufferHandler[capsuleIndex] = CapsuleColliderComponentGPU(capsule, entity);
            }
            };

        ForEachStream(capsulePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(capsulePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(capsulePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}