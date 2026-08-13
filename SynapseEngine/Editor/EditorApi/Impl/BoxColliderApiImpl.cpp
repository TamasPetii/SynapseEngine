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

#include "BoxColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"

namespace Syn {

    bool BoxColliderApiImpl::HasBoxCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<BoxColliderComponent>(_sceneManager, entity);
    }

    glm::vec3 BoxColliderApiImpl::GetBoxColliderHalfExtents(EntityID entity) const {
        return EditorApiUtils::ReadComponent<BoxColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.halfExtents; }, glm::vec3(0.5f));
    }

    glm::vec3 BoxColliderApiImpl::GetBoxColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<BoxColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void BoxColliderApiImpl::SetBoxColliderHalfExtents(EntityID entity, const glm::vec3& halfExtents) {
        EditorApiUtils::ModifyComponent<BoxColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.halfExtents = halfExtents; });
    }

    void BoxColliderApiImpl::SetBoxColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<BoxColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}