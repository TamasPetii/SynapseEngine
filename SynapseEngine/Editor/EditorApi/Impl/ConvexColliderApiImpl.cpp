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

#include "ConvexColliderApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Physics/ConvexColliderComponent.h"

namespace Syn {

    bool ConvexColliderApiImpl::HasConvexCollider(EntityID entity) const {
        return EditorApiUtils::HasComponent<ConvexColliderComponent>(_sceneManager, entity);
    }

    uint32_t ConvexColliderApiImpl::GetConvexColliderTargetLodLevel(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ConvexColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.targetLodLevel; }, 0u);
    }

    glm::vec3 ConvexColliderApiImpl::GetConvexColliderLocalOffset(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ConvexColliderComponent>(_sceneManager, entity, [](const auto& c) { return c.localOffset; }, glm::vec3(0.0f));
    }

    void ConvexColliderApiImpl::SetConvexColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) {
        EditorApiUtils::ModifyComponent<ConvexColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.targetLodLevel = targetLodLevel; });
    }

    void ConvexColliderApiImpl::SetConvexColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) {
        EditorApiUtils::ModifyComponent<ConvexColliderComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.localOffset = localOffset; });
    }
}