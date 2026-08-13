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

#include "DirectionLightApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"

namespace Syn {

    bool DirectionLightApiImpl::HasDirectionLight(EntityID entity) const {
        return EditorApiUtils::HasComponent<DirectionLightComponent>(_sceneManager, entity);
    }

    glm::vec3 DirectionLightApiImpl::GetLightColor(EntityID entity) const {
        return EditorApiUtils::ReadComponent<DirectionLightComponent>(_sceneManager, entity, [](const auto& c) { return c.color; }, glm::vec3(1.0f));
    }

    float DirectionLightApiImpl::GetLightStrength(EntityID entity) const {
        return EditorApiUtils::ReadComponent<DirectionLightComponent>(_sceneManager, entity, [](const auto& c) { return c.strength; }, 1.0f);
    }

    bool DirectionLightApiImpl::GetLightUseShadow(EntityID entity) const {
        return EditorApiUtils::ReadComponent<DirectionLightComponent>(_sceneManager, entity, [](const auto& c) { return c.useShadow; }, false);
    }

    void DirectionLightApiImpl::SetLightColor(EntityID entity, const glm::vec3& color) {
        EditorApiUtils::ModifyComponent<DirectionLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.color = color; });
    }

    void DirectionLightApiImpl::SetLightStrength(EntityID entity, float strength) {
        EditorApiUtils::ModifyComponent<DirectionLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.strength = strength; });
    }

    void DirectionLightApiImpl::SetLightUseShadow(EntityID entity, bool useShadow) {
        EditorApiUtils::ModifyComponent<DirectionLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            if (c.useShadow != useShadow) {
                c.useShadow = useShadow;
                pool->template SetBit<SHADOW_TOGGLE_BIT>(entity);
            }
        });
    }

    float DirectionLightApiImpl::GetShadowFarPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<DirectionLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.shadowFarPlane; }, 500.0f);
    }

    glm::vec4 DirectionLightApiImpl::GetCascadeSplits(EntityID entity) const {
        return EditorApiUtils::ReadComponent<DirectionLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.cascadeSplits; }, glm::vec4(0.075f, 0.20f, 0.50f, 1.0f));
    }

    void DirectionLightApiImpl::SetShadowFarPlane(EntityID entity, float farPlane) {
        EditorApiUtils::ModifyComponent<DirectionLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.shadowFarPlane = farPlane; });
    }

    void DirectionLightApiImpl::SetCascadeSplits(EntityID entity, const glm::vec4& splits) {
        EditorApiUtils::ModifyComponent<DirectionLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.cascadeSplits = splits; });
    }
}