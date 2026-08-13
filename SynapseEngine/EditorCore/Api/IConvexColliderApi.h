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
#include "IApi.h"
#include <glm/glm.hpp>
#include "EditorCore/Types/EntityHandle.h"
#include <cstdint>

namespace Syn {
    class IConvexColliderApi : public IApi {
    public:
        virtual ~IConvexColliderApi() = default;

        virtual bool HasConvexCollider(EntityID entity) const = 0;

        virtual uint32_t GetConvexColliderTargetLodLevel(EntityID entity) const = 0;
        virtual glm::vec3 GetConvexColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetConvexColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) = 0;
        virtual void SetConvexColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}