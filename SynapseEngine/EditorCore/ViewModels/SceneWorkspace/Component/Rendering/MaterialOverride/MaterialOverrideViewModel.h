#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialOverrideState.h"
#include "MaterialOverrideIntent.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IMaterialOverrideApi.h"

namespace Syn {
    class MaterialOverrideViewModel : public IViewModel<MaterialOverrideState, MaterialOverrideIntent> {
    public:
        MaterialOverrideViewModel(ISelectionApi* selectionApi, IMaterialOverrideApi* overrideApi);
        ~MaterialOverrideViewModel() override = default;

        const MaterialOverrideState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const MaterialOverrideIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        IMaterialOverrideApi* _overrideApi = nullptr;
        MaterialOverrideState _state;
    };
}