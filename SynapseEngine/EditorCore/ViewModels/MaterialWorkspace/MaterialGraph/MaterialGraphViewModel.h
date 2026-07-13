#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialGraphState.h"
#include "MaterialGraphIntent.h"
#include "EditorCore/Api/IMaterialApi.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class MaterialGraphViewModel : public IViewModel<MaterialGraphState, MaterialGraphIntent> {
    public:
        MaterialGraphViewModel(IMaterialApi* materialApi, ITextureApi* textureApi);

        const MaterialGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const MaterialGraphIntent& intent) override;
    private:
        void RebuildGraphForSelectedMaterial(uint32_t materialId);
        void HandleCreateLink(const CreateLinkIntent& intent);
        void HandleDeleteLink(const DeleteLinkIntent& intent);
    private:
        IMaterialApi* _materialApi = nullptr;
        ITextureApi* _textureApi = nullptr;

        MaterialGraphState _state;

        uint32_t _lastSelectedMaterial = INVALID_MATERIAL_ID;
        uint64_t _lastEngineVersion = 0;
    };
}