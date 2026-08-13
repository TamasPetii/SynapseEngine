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
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ICameraApi : public IApi {
    public:
        virtual ~ICameraApi() = default;

        virtual bool HasCamera(EntityID entity) const = 0;

        virtual bool GetCameraIsOrthographic(EntityID entity) const = 0;
        virtual float GetCameraOrthoSize(EntityID entity) const = 0;
        virtual bool GetCameraUseOrbit(EntityID entity) const = 0;

        virtual float GetCameraYaw(EntityID entity) const = 0;
        virtual float GetCameraPitch(EntityID entity) const = 0;
        virtual float GetCameraNearPlane(EntityID entity) const = 0;
        virtual float GetCameraFarPlane(EntityID entity) const = 0;
        virtual float GetCameraFov(EntityID entity) const = 0;
        virtual float GetCameraSpeed(EntityID entity) const = 0;
        virtual float GetCameraSensitivity(EntityID entity) const = 0;
        virtual float GetCameraDistance(EntityID entity) const = 0;

        virtual void SetCameraIsOrthographic(EntityID entity, bool isOrthographic) = 0;
        virtual void SetCameraOrthoSize(EntityID entity, float orthoSize) = 0;
        virtual void SetCameraUseOrbit(EntityID entity, bool useOrbit) = 0;

        virtual void SetCameraYaw(EntityID entity, float yaw) = 0;
        virtual void SetCameraPitch(EntityID entity, float pitch) = 0;
        virtual void SetCameraNearPlane(EntityID entity, float nearPlane) = 0;
        virtual void SetCameraFarPlane(EntityID entity, float farPlane) = 0;
        virtual void SetCameraFov(EntityID entity, float fov) = 0;
        virtual void SetCameraSpeed(EntityID entity, float speed) = 0;
        virtual void SetCameraSensitivity(EntityID entity, float sensitivity) = 0;
        virtual void SetCameraDistance(EntityID entity, float distance) = 0;
    };
}