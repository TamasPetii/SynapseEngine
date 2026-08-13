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

#include "TransformApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn {
    glm::vec3 TransformApiImpl::GetEntityPosition(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.translation; }, glm::vec3(0.0f));
    }

    void TransformApiImpl::SetEntityPosition(EntityID entity, const glm::vec3& position) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.translation = position; 
            pool->template SetBit<TRANSFORM_POS_CHANGED>(entity); 
        });
    }

    glm::vec3 TransformApiImpl::GetEntityRotation(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.rotation; }, glm::vec3(0.0f));
    }

    void TransformApiImpl::SetEntityRotation(EntityID entity, const glm::vec3& rotation) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.rotation = rotation; 
            pool->template SetBit<TRANSFORM_ROT_CHANGED>(entity); 
        });
    }

    glm::vec3 TransformApiImpl::GetEntityScale(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.scale; }, glm::vec3(1.0f));
    }

    void TransformApiImpl::SetEntityScale(EntityID entity, const glm::vec3& scale) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.scale = scale; 
            pool->template SetBit<TRANSFORM_SCALE_CHANGED>(entity); 
        });
    }

    glm::mat4 TransformApiImpl::GetEntityWorldMatrix(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.transform; }, glm::mat4(1.0f));
    }
}