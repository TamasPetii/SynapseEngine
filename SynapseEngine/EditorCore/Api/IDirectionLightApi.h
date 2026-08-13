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

namespace Syn {
    class IDirectionLightApi : public IApi {
    public:
        virtual ~IDirectionLightApi() = default;

        virtual bool HasDirectionLight(EntityID entity) const = 0;

        virtual glm::vec3 GetLightColor(EntityID entity) const = 0;
        virtual float GetLightStrength(EntityID entity) const = 0;
        virtual bool GetLightUseShadow(EntityID entity) const = 0;

        virtual void SetLightColor(EntityID entity, const glm::vec3& color) = 0;
        virtual void SetLightStrength(EntityID entity, float strength) = 0;
        virtual void SetLightUseShadow(EntityID entity, bool useShadow) = 0;

        virtual float GetShadowFarPlane(EntityID entity) const = 0;
        virtual glm::vec4 GetCascadeSplits(EntityID entity) const = 0;

        virtual void SetShadowFarPlane(EntityID entity, float farPlane) = 0;
        virtual void SetCascadeSplits(EntityID entity, const glm::vec4& splits) = 0;
    };
}