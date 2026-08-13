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

#include "TagApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn {
    std::string TagApiImpl::GetEntityTag(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.tag; }, std::string("Untagged"));
    }

    void TagApiImpl::SetEntityTag(EntityID entity, const std::string& tag) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.tag = tag; });
    }

    std::string TagApiImpl::GetEntityName(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.name; }, "Entity " + std::to_string(entity));
    }

    void TagApiImpl::SetEntityName(EntityID entity, const std::string& name) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.name = name; });
    }

    bool TagApiImpl::IsEntityEnabled(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.localEnabled; }, true);
    }

    void TagApiImpl::SetEntityEnabled(EntityID entity, bool enabled) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.localEnabled = enabled; });
    }
}