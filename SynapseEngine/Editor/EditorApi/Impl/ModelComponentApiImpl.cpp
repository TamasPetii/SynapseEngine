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

#include "ModelComponentApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"

namespace Syn {
    bool ModelComponentApiImpl::HasModelComponent(EntityID entity) const {
        return EditorApiUtils::HasComponent<ModelComponent>(_sceneManager, entity);
    }

    bool ModelComponentApiImpl::GetCastShadow(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ModelComponent>(_sceneManager, entity, [](const auto& c) { return c.castShadow; }, true);
    }

    bool ModelComponentApiImpl::GetReceiveShadow(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ModelComponent>(_sceneManager, entity, [](const auto& c) { return c.receiveShadow; }, true);
    }

    uint32_t ModelComponentApiImpl::GetModelIndex(EntityID entity) const {
        return EditorApiUtils::ReadComponent<ModelComponent>(_sceneManager, entity, [](const auto& c) { return c.modelIndex; }, UINT32_MAX);
    }

    void ModelComponentApiImpl::SetCastShadow(EntityID entity, bool cast) {
        EditorApiUtils::ModifyComponent<ModelComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.castShadow = cast; });
    }

    void ModelComponentApiImpl::SetReceiveShadow(EntityID entity, bool receive) {
        EditorApiUtils::ModifyComponent<ModelComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.receiveShadow = receive; });
    }

    void ModelComponentApiImpl::SetModelIndex(EntityID entity, uint32_t index) {
        EditorApiUtils::ModifyComponent<ModelComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.modelIndex = index; });
    }

    std::vector<std::pair<uint32_t, std::string>> ModelComponentApiImpl::GetAvailableModels() const {
        std::vector<std::pair<uint32_t, std::string>> result;

        auto modelManager = ServiceLocator::Get<ModelManager>();
        if (!modelManager) return result;

        auto paths = modelManager->GetResourcePaths();
        auto snapshots = modelManager->GetResourceSnapshot();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (snapshots[i].state == ResourceState::Ready) {
                result.push_back({ i, paths[i] });
            }
        }

        return result;
    }
}