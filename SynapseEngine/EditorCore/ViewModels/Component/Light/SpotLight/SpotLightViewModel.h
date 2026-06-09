#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "SpotLightState.h"
#include "SpotLightIntent.h"
#include "SpotLightCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ISpotLightApi.h"

namespace Syn {
    class SpotLightViewModel : public IViewModel<SpotLightState, SpotLightIntent> {
    public:
        SpotLightViewModel(ISelectionApi* selectionApi, ISpotLightApi* lightApi);
        ~SpotLightViewModel() override = default;

        const SpotLightState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const SpotLightIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        ISpotLightApi* _lightApi = nullptr;
        SpotLightState _state;

        DragInteraction<glm::vec3> _colorDrag;
        DragInteraction<float> _strengthDrag;
        DragInteraction<float> _rangeDrag;
        DragInteraction<float> _weakenDrag;
        DragInteraction<float> _innerAngleDrag;
        DragInteraction<float> _outerAngleDrag;
        DragInteraction<float> _nearPlaneDrag;
        DragInteraction<float> _farPlaneDrag;
    };
}