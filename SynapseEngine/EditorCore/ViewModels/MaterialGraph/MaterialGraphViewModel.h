#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialGraphState.h"
#include "MaterialGraphIntent.h"
#include "EditorCore/Api/IMaterialAPI.h"


namespace Syn {
    class MaterialGraphViewModel : public IViewModel<MaterialGraphState, MaterialGraphIntent> {
    public:
        MaterialGraphViewModel(IMaterialAPI* materialApi);

        const MaterialGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const MaterialGraphIntent& intent) override;
    private:
        void BuildGraphFromEngine();
        void HandleCreateLink(const CreateLinkIntent& intent);
        void HandleDeleteLink(const DeleteLinkIntent& intent);
    private:
        IMaterialAPI* _materialApi = nullptr;
        MaterialGraphState _state;
    };
}