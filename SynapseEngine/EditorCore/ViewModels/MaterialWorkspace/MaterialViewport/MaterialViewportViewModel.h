#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialViewportState.h"
#include "MaterialViewportIntent.h"
#include "EditorCore/Api/IRenderApi.h"

namespace Syn {
    class MaterialViewportViewModel : public IViewModel<MaterialViewportState, MaterialViewportIntent> {
    public:
        explicit MaterialViewportViewModel(IRenderApi* renderApi);
        ~MaterialViewportViewModel() override = default;

        const MaterialViewportState& GetState() const override;

        void SyncWithEngine() override;
        void Dispatch(const MaterialViewportIntent& intent) override;

    private:
        void HandleResize(const ResizeMaterialViewportIntent& intent);

    private:
        IRenderApi* _renderApi = nullptr;
        MaterialViewportState _state;
    };
}