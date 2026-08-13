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
#include <cstdint>

namespace Syn {
    class IMeshColliderApi : public IApi {
    public:
        virtual ~IMeshColliderApi() = default;

        virtual bool HasMeshCollider(EntityID entity) const = 0;

        virtual uint32_t GetMeshColliderTargetLodLevel(EntityID entity) const = 0;
        virtual glm::vec3 GetMeshColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) = 0;
        virtual void SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}