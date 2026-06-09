#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "DirectionLightState.h"
#include "DirectionLightIntent.h"
#include "DirectionLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"

namespace Syn {
    class DirectionLightViewModel : public IViewModel<DirectionLightState, DirectionLightIntent> {
    public:
        DirectionLightViewModel(ISelectionApi* selectionApi, IDirectionLightApi* lightApi);
        ~DirectionLightViewModel() override = default;

        const DirectionLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const DirectionLightIntent& intent) override;

    private:
        void HandleSetColor(const SetLightColorIntent& intent);
        void HandleSetStrength(const SetLightStrengthIntent& intent);
        void HandleSetUseShadow(const SetLightUseShadowIntent& intent);
        void HandleSetFarPlane(const SetShadowFarPlaneIntent& intent);
        void HandleSetCascadeSplits(const SetCascadeSplitsIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IDirectionLightApi* _lightApi = nullptr;
        DirectionLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _farPlaneDrag;
        DragInteraction<glm::vec4> _cascadeSplitsDrag;
    };
}