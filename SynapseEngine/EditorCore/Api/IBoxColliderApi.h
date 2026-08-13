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
    class IBoxColliderApi : public IApi {
    public:
        virtual ~IBoxColliderApi() = default;

        virtual bool HasBoxCollider(EntityID entity) const = 0;

        virtual glm::vec3 GetBoxColliderHalfExtents(EntityID entity) const = 0;
        virtual glm::vec3 GetBoxColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetBoxColliderHalfExtents(EntityID entity, const glm::vec3& halfExtents) = 0;
        virtual void SetBoxColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}