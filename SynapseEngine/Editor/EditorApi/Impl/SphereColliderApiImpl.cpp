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

#include "SphereColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"

namespace Syn {

    bool SphereColliderApiImpl::HasSphereCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<SphereColliderComponent>(_sceneManager, entity);
    }

    float SphereColliderApiImpl::GetSphereColliderRadius(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SphereColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.radius; }, 0.5f);
    }

    glm::vec3 SphereColliderApiImpl::GetSphereColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SphereColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void SphereColliderApiImpl::SetSphereColliderRadius(EntityID entity, float radius) {
        EditorApiUtils::ModifyComponent<SphereColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.radius = radius; });
    }

    void SphereColliderApiImpl::SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<SphereColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}