#include "CameraViewModel.h"

namespace Syn
{
    CameraViewModel::CameraViewModel(ISelectionApi* selectionApi, ICameraApi* cameraApi)
        : _selectionApi(selectionApi), _cameraApi(cameraApi)
    {}

    const CameraState& CameraViewModel::GetState() const
    {
        return _state;
    }

    void CameraViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_cameraApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _cameraApi->HasCamera(activeEntity))
        {
            _state.hasComponent = true;

            _state.isOrthographic = _cameraApi->GetCameraIsOrthographic(activeEntity);
            _state.useOrbit = _cameraApi->GetCameraUseOrbit(activeEntity);

            if (!_orthoSizeDrag.IsDragging())   _state.orthoSize = _cameraApi->GetCameraOrthoSize(activeEntity);

            if (!_yawDrag.IsDragging())         _state.yaw = _cameraApi->GetCameraYaw(activeEntity);
            if (!_pitchDrag.IsDragging())       _state.pitch = _cameraApi->GetCameraPitch(activeEntity);
            if (!_nearPlaneDrag.IsDragging())   _state.nearPlane = _cameraApi->GetCameraNearPlane(activeEntity);
            if (!_farPlaneDrag.IsDragging())    _state.farPlane = _cameraApi->GetCameraFarPlane(activeEntity);
            if (!_fovDrag.IsDragging())         _state.fov = _cameraApi->GetCameraFov(activeEntity);
            if (!_speedDrag.IsDragging())       _state.speed = _cameraApi->GetCameraSpeed(activeEntity);
            if (!_sensitivityDrag.IsDragging()) _state.sensitivity = _cameraApi->GetCameraSensitivity(activeEntity);
            if (!_distanceDrag.IsDragging())    _state.distance = _cameraApi->GetCameraDistance(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void CameraViewModel::Dispatch(const CameraIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                // Új Dispatchek
                if constexpr (std::is_same_v<T, SetCameraIsOrthographicIntent>)   HandleSetIsOrthographic(arg);
                else if constexpr (std::is_same_v<T, SetCameraOrthoSizeIntent>)   HandleSetOrthoSize(arg);
                else if constexpr (std::is_same_v<T, SetCameraUseOrbitIntent>)    HandleSetUseOrbit(arg);

                else if constexpr (std::is_same_v<T, SetCameraYawIntent>)         HandleSetYaw(arg);
                else if constexpr (std::is_same_v<T, SetCameraPitchIntent>)       HandleSetPitch(arg);
                else if constexpr (std::is_same_v<T, SetCameraNearPlaneIntent>)   HandleSetNearPlane(arg);
                else if constexpr (std::is_same_v<T, SetCameraFarPlaneIntent>)    HandleSetFarPlane(arg);
                else if constexpr (std::is_same_v<T, SetCameraFovIntent>)         HandleSetFov(arg);
                else if constexpr (std::is_same_v<T, SetCameraSpeedIntent>)       HandleSetSpeed(arg);
                else if constexpr (std::is_same_v<T, SetCameraSensitivityIntent>) HandleSetSensitivity(arg);
                else if constexpr (std::is_same_v<T, SetCameraDistanceIntent>)    HandleSetDistance(arg);
            }, intent);
    }

    void CameraViewModel::HandleSetIsOrthographic(const SetCameraIsOrthographicIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.isOrthographic = intent.isOrthographic;
        _cameraApi->SetCameraIsOrthographic(activeEntity, intent.isOrthographic);
    }

    void CameraViewModel::HandleSetUseOrbit(const SetCameraUseOrbitIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.useOrbit = intent.useOrbit;
        _cameraApi->SetCameraUseOrbit(activeEntity, intent.useOrbit);
    }

    void CameraViewModel::HandleSetOrthoSize(const SetCameraOrthoSizeIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _orthoSizeDrag.Handle(intent.isDragging, intent.orthoSize, _state.orthoSize,
            [&](const float& v) { _cameraApi->SetCameraOrthoSize(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraOrthoSizeCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetYaw(const SetCameraYawIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _yawDrag.Handle(intent.isDragging, intent.yaw, _state.yaw,
            [&](const float& v) { _cameraApi->SetCameraYaw(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraYawCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetPitch(const SetCameraPitchIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _pitchDrag.Handle(intent.isDragging, intent.pitch, _state.pitch,
            [&](const float& v) { _cameraApi->SetCameraPitch(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraPitchCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetNearPlane(const SetCameraNearPlaneIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _nearPlaneDrag.Handle(intent.isDragging, intent.nearPlane, _state.nearPlane,
            [&](const float& v) { _cameraApi->SetCameraNearPlane(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraNearPlaneCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetFarPlane(const SetCameraFarPlaneIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _farPlaneDrag.Handle(intent.isDragging, intent.farPlane, _state.farPlane,
            [&](const float& v) { _cameraApi->SetCameraFarPlane(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraFarPlaneCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetFov(const SetCameraFovIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _fovDrag.Handle(intent.isDragging, intent.fov, _state.fov,
            [&](const float& v) { _cameraApi->SetCameraFov(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraFovCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetSpeed(const SetCameraSpeedIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _speedDrag.Handle(intent.isDragging, intent.speed, _state.speed,
            [&](const float& v) { _cameraApi->SetCameraSpeed(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraSpeedCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetSensitivity(const SetCameraSensitivityIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _sensitivityDrag.Handle(intent.isDragging, intent.sensitivity, _state.sensitivity,
            [&](const float& v) { _cameraApi->SetCameraSensitivity(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraSensitivityCommand>(_cameraApi, activeEntity, s, e); });
    }

    void CameraViewModel::HandleSetDistance(const SetCameraDistanceIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;
        _distanceDrag.Handle(intent.isDragging, intent.distance, _state.distance,
            [&](const float& v) { _cameraApi->SetCameraDistance(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCameraDistanceCommand>(_cameraApi, activeEntity, s, e); });
    }
}