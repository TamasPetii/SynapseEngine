#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "PointLightState.h"
#include "PointLightIntent.h"
#include "PointLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IPointLightApi.h"

namespace Syn {
    class PointLightViewModel : public IViewModel<PointLightState, PointLightIntent> {
    public:
        PointLightViewModel(ISelectionApi* selectionApi, IPointLightApi* lightApi);
        ~PointLightViewModel() override = default;

        const PointLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const PointLightIntent& intent) override;

    private:
        void HandleSetColor(const SetPointLightColorIntent& intent);
        void HandleSetStrength(const SetPointLightStrengthIntent& intent);
        void HandleSetUseShadow(const SetPointLightUseShadowIntent& intent);
        void HandleSetRadius(const SetPointLightRadiusIntent& intent);
        void HandleSetWeaken(const SetPointLightWeakenIntent& intent);
        void HandleSetNearPlane(const SetPointLightShadowNearIntent& intent);
        void HandleSetFarPlane(const SetPointLightShadowFarIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IPointLightApi* _lightApi = nullptr;
        PointLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _radiusDrag;
        DragInteraction<float> _weakenDrag;
        DragInteraction<float> _nearPlaneDrag;
        DragInteraction<float> _farPlaneDrag;
    };
}