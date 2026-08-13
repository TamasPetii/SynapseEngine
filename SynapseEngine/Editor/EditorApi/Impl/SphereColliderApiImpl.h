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
#include "EditorCore/Api/ISphereColliderApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SphereColliderApiImpl : public ISphereColliderApi {
    public:
        SphereColliderApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasSphereCollider(EntityID entity) const override;

        float GetSphereColliderRadius(EntityID entity) const override;
        glm::vec3 GetSphereColliderLocalOffset(EntityID entity) const override;

        void SetSphereColliderRadius(EntityID entity, float radius) override;
        void SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) override;

    private:
        SceneManager* _sceneManager;
    };
}