#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/ICameraApi.h"

namespace Syn
{
    using ChangeCameraIsOrthographicCommand = ComponentChangeCommand<ICameraApi, bool, &ICameraApi::SetCameraIsOrthographic>;
    using ChangeCameraOrthoSizeCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraOrthoSize>;
    using ChangeCameraUseOrbitCommand = ComponentChangeCommand<ICameraApi, bool, &ICameraApi::SetCameraUseOrbit>;
    using ChangeCameraYawCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraYaw>;
    using ChangeCameraPitchCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraPitch>;
    using ChangeCameraNearPlaneCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraNearPlane>;
    using ChangeCameraFarPlaneCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraFarPlane>;
    using ChangeCameraFovCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraFov>;
    using ChangeCameraSpeedCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraSpeed>;
    using ChangeCameraSensitivityCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraSensitivity>;
    using ChangeCameraDistanceCommand = ComponentChangeCommand<ICameraApi, float, &ICameraApi::SetCameraDistance>;
}