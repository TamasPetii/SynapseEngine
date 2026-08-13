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
#include "EditorCore/Api/ISpotLightApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SpotLightApiImpl : public ISpotLightApi {
    public:
        SpotLightApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasSpotLight(EntityID entity) const override;
        glm::vec3 GetLightColor(EntityID entity) const override;
        float GetLightStrength(EntityID entity) const override;
        bool GetLightUseShadow(EntityID entity) const override;
        float GetLightRange(EntityID entity) const override;
        float GetLightWeakenDistance(EntityID entity) const override;
        float GetLightInnerAngle(EntityID entity) const override;
        float GetLightOuterAngle(EntityID entity) const override;

        void SetLightColor(EntityID entity, const glm::vec3& color) override;
        void SetLightStrength(EntityID entity, float strength) override;
        void SetLightUseShadow(EntityID entity, bool useShadow) override;
        void SetLightRange(EntityID entity, float range) override;
        void SetLightWeakenDistance(EntityID entity, float distance) override;
        void SetLightInnerAngle(EntityID entity, float angle) override;
        void SetLightOuterAngle(EntityID entity, float angle) override;

        float GetShadowNearPlane(EntityID entity) const override;
        float GetShadowFarPlane(EntityID entity) const override;
        void SetShadowNearPlane(EntityID entity, float nearPlane) override;
        void SetShadowFarPlane(EntityID entity, float farPlane) override;
    private:
        SceneManager* _sceneManager;
    };
}