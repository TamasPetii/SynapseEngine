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

#include "SphereColliderSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "PhysicsUtils.h"

#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Core/TransformSystem.h"

namespace Syn
{
    std::vector<TypeID> SphereColliderSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    std::vector<TypeID> SphereColliderSystem::GetWriteDependencies() const
    {
        return { TypeInfo<RigidBodySystem>::ID, TypeInfo<SphereColliderSystem>::ID };
    }

    void SphereColliderSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto spherePool = registry->GetPool<SphereColliderComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto physicsEngine = scene->GetPhysicsEngine();

        if (!spherePool || !rbPool || !transformPool || !physicsEngine) return;

        ParallelForEach(spherePool, subflow, SystemPhaseNames::Update, [spherePool, rbPool, transformPool, physicsEngine](EntityID entity) {
            if (!rbPool->Has(entity) || !transformPool->Has(entity)) return;

            auto& sphere = spherePool->Get(entity);
            auto& rb = rbPool->Get(entity);
            auto& tr = transformPool->Get(entity);

            if (rb.bodyID == INVALID_BODY_ID)
            {
                rb.bodyID = PhysicsUtils::TryCreateBody(entity, &tr, rb,
                    [&](const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const PhysicsBodySettings& settings) {
                        return physicsEngine->CreateSphereBody(pos, rot, sphere.radius * glm::max(scale.x, glm::max(scale.y, scale.z)), settings);
                    });
            }
            else if (spherePool->IsBitSet<UPDATE_BIT>(entity))
            {
                glm::vec3 worldScale = PhysicsUtils::ExtractScale(tr.transform);
                physicsEngine->SetSphereShape(rb.bodyID, sphere.radius * glm::max(worldScale.x, glm::max(worldScale.y, worldScale.z)));
            }

            sphere.version++;
            });
    }

    void SphereColliderSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto spherePool = registry->GetPool<SphereColliderComponent>();
        if (!spherePool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::SphereColliderData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<SphereColliderComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [spherePool, bufferHandler, componentBuffer](EntityID entity) {
            auto& sphere = spherePool->Get(entity);
            auto sphereIndex = spherePool->GetMapping().Get(entity);

            if (componentBuffer.versions[sphereIndex] != sphere.version)
            {
                componentBuffer.versions[sphereIndex] = sphere.version;
				bufferHandler[sphereIndex] = SphereColliderComponentGPU(sphere, entity);
            }
            };

        ForEachStream(spherePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(spherePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(spherePool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}