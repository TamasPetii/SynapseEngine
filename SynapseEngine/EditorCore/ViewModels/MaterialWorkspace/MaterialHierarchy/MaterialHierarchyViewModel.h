#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialHierarchyState.h"
#include "MaterialHierarchyIntent.h"
#include "EditorCore/Api/IPreviewApi.h"
#include "EditorCore/Api/IMaterialApi.h"

namespace Syn {
    class MaterialHierarchyViewModel : public IViewModel<MaterialHierarchyState, MaterialHierarchyIntent> {
    public:
        MaterialHierarchyViewModel(IMaterialApi* materialApi, IPreviewApi* previewApi);
        ~MaterialHierarchyViewModel() override = default;

        const MaterialHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const MaterialHierarchyIntent& intent) override;

    private:
        void RebuildList();
    private:
        IMaterialApi* _materialApi = nullptr;
        IPreviewApi* _previewApi = nullptr;

        MaterialHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
    };
}