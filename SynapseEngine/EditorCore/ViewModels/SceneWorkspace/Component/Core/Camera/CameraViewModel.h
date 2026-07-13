#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "CameraState.h"
#include "CameraIntent.h"
#include "CameraCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ICameraApi.h"

namespace Syn {
    class CameraViewModel : public IViewModel<CameraState, CameraIntent> {
    public:
        CameraViewModel(ISelectionApi* selectionApi, ICameraApi* cameraApi);
        ~CameraViewModel() override = default;

        const CameraState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const CameraIntent& intent) override;

    private:
        void HandleSetIsOrthographic(const SetCameraIsOrthographicIntent& intent);
        void HandleSetOrthoSize(const SetCameraOrthoSizeIntent& intent);
        void HandleSetUseOrbit(const SetCameraUseOrbitIntent& intent);
        void HandleSetYaw(const SetCameraYawIntent& intent);
        void HandleSetPitch(const SetCameraPitchIntent& intent);
        void HandleSetNearPlane(const SetCameraNearPlaneIntent& intent);
        void HandleSetFarPlane(const SetCameraFarPlaneIntent& intent);
        void HandleSetFov(const SetCameraFovIntent& intent);
        void HandleSetSpeed(const SetCameraSpeedIntent& intent);
        void HandleSetSensitivity(const SetCameraSensitivityIntent& intent);
        void HandleSetDistance(const SetCameraDistanceIntent& intent);
    private:
        ISelectionApi* _selectionApi = nullptr;
        ICameraApi* _cameraApi = nullptr;
        CameraState _state;

        DragInteraction<float> _orthoSizeDrag;
        DragInteraction<float> _yawDrag;
        DragInteraction<float> _pitchDrag;
        DragInteraction<float> _nearPlaneDrag;
        DragInteraction<float> _farPlaneDrag;
        DragInteraction<float> _fovDrag;
        DragInteraction<float> _speedDrag;
        DragInteraction<float> _sensitivityDrag;
        DragInteraction<float> _distanceDrag;
    };
}