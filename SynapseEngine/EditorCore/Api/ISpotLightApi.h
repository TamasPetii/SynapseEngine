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
    class ISpotLightApi : public IApi {
    public:
        virtual ~ISpotLightApi() = default;

        virtual bool HasSpotLight(EntityID entity) const = 0;

        virtual glm::vec3 GetLightColor(EntityID entity) const = 0;
        virtual float GetLightStrength(EntityID entity) const = 0;
        virtual bool GetLightUseShadow(EntityID entity) const = 0;
        
        virtual float GetLightRange(EntityID entity) const = 0;
        virtual float GetLightWeakenDistance(EntityID entity) const = 0;
        virtual float GetLightInnerAngle(EntityID entity) const = 0;
        virtual float GetLightOuterAngle(EntityID entity) const = 0;

        virtual void SetLightColor(EntityID entity, const glm::vec3& color) = 0;
        virtual void SetLightStrength(EntityID entity, float strength) = 0;
        virtual void SetLightUseShadow(EntityID entity, bool useShadow) = 0;
        
        virtual void SetLightRange(EntityID entity, float range) = 0;
        virtual void SetLightWeakenDistance(EntityID entity, float distance) = 0;
        virtual void SetLightInnerAngle(EntityID entity, float angle) = 0;
        virtual void SetLightOuterAngle(EntityID entity, float angle) = 0;

        virtual float GetShadowNearPlane(EntityID entity) const = 0;
        virtual float GetShadowFarPlane(EntityID entity) const = 0;

        virtual void SetShadowNearPlane(EntityID entity, float nearPlane) = 0;
        virtual void SetShadowFarPlane(EntityID entity, float farPlane) = 0;
    };
}