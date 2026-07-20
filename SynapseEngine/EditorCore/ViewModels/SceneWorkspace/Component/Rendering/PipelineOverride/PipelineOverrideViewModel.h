#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "PipelineOverrideState.h"
#include "PipelineOverrideIntent.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IPipelineOverrideApi.h"

namespace Syn {
    class PipelineOverrideViewModel : public IViewModel<PipelineOverrideState, PipelineOverrideIntent> {
    public:
        PipelineOverrideViewModel(ISelectionApi* selectionApi, IPipelineOverrideApi* overrideApi);
        ~PipelineOverrideViewModel() override = default;

        const PipelineOverrideState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const PipelineOverrideIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IPipelineOverrideApi* _overrideApi = nullptr;
        PipelineOverrideState _state;
    };
}