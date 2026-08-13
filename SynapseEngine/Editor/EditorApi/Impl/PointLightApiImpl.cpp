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

#include "PointLightApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Light/Point/PointLightShadowComponent.h"

namespace Syn {

    bool PointLightApiImpl::HasPointLight(EntityID entity) const {
        return EditorApiUtils::HasComponent<PointLightComponent>(_sceneManager, entity);
    }

    glm::vec3 PointLightApiImpl::GetLightColor(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightComponent>(_sceneManager, entity, [](const auto& c) { return c.color; }, glm::vec3(1.0f));
    }

    float PointLightApiImpl::GetLightStrength(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightComponent>(_sceneManager, entity, [](const auto& c) { return c.strength; }, 1.0f);
    }

    bool PointLightApiImpl::GetLightUseShadow(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightComponent>(_sceneManager, entity, [](const auto& c) { return c.useShadow; }, false);
    }

    float PointLightApiImpl::GetLightRadius(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightComponent>(_sceneManager, entity, [](const auto& c) { return c.radius; }, 10.0f);
    }

    float PointLightApiImpl::GetLightWeakenDistance(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightComponent>(_sceneManager, entity, [](const auto& c) { return c.weakenDistance; }, 0.0f);
    }

    void PointLightApiImpl::SetLightColor(EntityID entity, const glm::vec3& color) {
        EditorApiUtils::ModifyComponent<PointLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.color = color; });
    }

    void PointLightApiImpl::SetLightStrength(EntityID entity, float strength) {
        EditorApiUtils::ModifyComponent<PointLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.strength = strength; });
    }

    void PointLightApiImpl::SetLightRadius(EntityID entity, float radius) {
        EditorApiUtils::ModifyComponent<PointLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.radius = radius; });
    }

    void PointLightApiImpl::SetLightWeakenDistance(EntityID entity, float distance) {
        EditorApiUtils::ModifyComponent<PointLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.weakenDistance = distance; });
    }

    void PointLightApiImpl::SetLightUseShadow(EntityID entity, bool useShadow) {
        EditorApiUtils::ModifyComponent<PointLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            if (c.useShadow != useShadow) {
                c.useShadow = useShadow;
                pool->template SetBit<SHADOW_TOGGLE_BIT>(entity);
            }
        });
    }

    float PointLightApiImpl::GetShadowNearPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.nearPlane; }, 0.1f);
    }

    float PointLightApiImpl::GetShadowFarPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PointLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.farPlane; }, 10.0f);
    }

    void PointLightApiImpl::SetShadowNearPlane(EntityID entity, float nearPlane) {
        EditorApiUtils::ModifyComponent<PointLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.nearPlane = nearPlane; });
    }

    void PointLightApiImpl::SetShadowFarPlane(EntityID entity, float farPlane) {
        EditorApiUtils::ModifyComponent<PointLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.farPlane = farPlane; });
    }
}