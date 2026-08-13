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
#include <glm/glm.hpp>
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ISphereColliderApi : public IApi {
    public:
        virtual ~ISphereColliderApi() = default;

        virtual bool HasSphereCollider(EntityID entity) const = 0;

        virtual float GetSphereColliderRadius(EntityID entity) const = 0;
        virtual glm::vec3 GetSphereColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetSphereColliderRadius(EntityID entity, float radius) = 0;
        virtual void SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}