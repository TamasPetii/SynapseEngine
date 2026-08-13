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

#pragma once
#include "EditorCore/Api/IHierarchyApi.h"
#include "Engine/Scene/SceneManager.h"


namespace Syn {
    class HierarchyApiImpl : public IHierarchyApi {
    public:
        HierarchyApiImpl(SceneManager* sm, ModelManager* modelManager) : _sceneManager(sm), _modelManager(modelManager) {}

        uint64_t GetVersion() const override;
        std::vector<EntityID> GetRootEntities() const override;
        std::vector<EntityID> GetChildren(EntityID entity) const override;
        std::string GetEntityIcon(EntityID entity) const override;
        bool HasChildren(EntityID entity) const override;
        EntityID GetParent(EntityID entity) const override;
        void SetParent(EntityID child, EntityID parent) override;
        EntityID CreateEntity(EntityTemplate templateType, EntityID parent = NULL_ENTITY) override;
        void DestroyEntityRecursive(EntityID entity) override;
        void DestroyEntityKeepChildren(EntityID entity) override;
        EntityID CopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) override;
        EntityID FullCopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) override;
        void DestroyEntity(EntityID entity) override;
    private:
        SceneManager* _sceneManager;
        ModelManager* _modelManager;
    };
}