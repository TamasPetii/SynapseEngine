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
#include "EditorCore/Api/IMeshColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class MeshColliderApiImpl : public IMeshColliderApi {
    public:
        MeshColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasMeshCollider(EntityID entity) const override;

        uint32_t GetMeshColliderTargetLodLevel(EntityID entity) const override;
        glm::vec3 GetMeshColliderLocalOffset(EntityID entity) const override;

        void SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) override;
        void SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}