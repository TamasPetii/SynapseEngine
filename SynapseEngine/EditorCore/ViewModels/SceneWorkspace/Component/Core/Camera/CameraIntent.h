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