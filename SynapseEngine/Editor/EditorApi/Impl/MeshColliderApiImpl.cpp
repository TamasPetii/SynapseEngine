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

#include "MeshColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"

namespace Syn {

    bool MeshColliderApiImpl::HasMeshCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<MeshColliderComponent>(_sceneManager, entity);
    }

    uint32_t MeshColliderApiImpl::GetMeshColliderTargetLodLevel(EntityID entity) const {
        return EditorApiUtils::ReadComponent<MeshColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.targetLodLevel; }, 0u);
    }

    glm::vec3 MeshColliderApiImpl::GetMeshColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<MeshColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void MeshColliderApiImpl::SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) {
        EditorApiUtils::ModifyComponent<MeshColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.targetLodLevel = targetLodLevel; });
    }

    void MeshColliderApiImpl::SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<MeshColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}