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
#include <variant>

namespace Syn
{
    struct SetCameraIsOrthographicIntent { bool isOrthographic; };
    struct SetCameraOrthoSizeIntent { float orthoSize; bool isDragging; };
    struct SetCameraUseOrbitIntent { bool useOrbit; };

    struct SetCameraYawIntent { float yaw; bool isDragging; };
    struct SetCameraPitchIntent { float pitch; bool isDragging; };
    struct SetCameraNearPlaneIntent { float nearPlane; bool isDragging; };
    struct SetCameraFarPlaneIntent { float farPlane; bool isDragging; };
    struct SetCameraFovIntent { float fov; bool isDragging; };
    struct SetCameraSpeedIntent { float speed; bool isDragging; };
    struct SetCameraSensitivityIntent { float sensitivity; bool isDragging; };
    struct SetCameraDistanceIntent { float distance; bool isDragging; };

    using CameraIntent = std::variant<
        SetCameraIsOrthographicIntent,
        SetCameraOrthoSizeIntent,
        SetCameraUseOrbitIntent,
        SetCameraYawIntent,
        SetCameraPitchIntent,
        SetCameraNearPlaneIntent,
        SetCameraFarPlaneIntent,
        SetCameraFovIntent,
        SetCameraSpeedIntent,
        SetCameraSensitivityIntent,
        SetCameraDistanceIntent>;
}