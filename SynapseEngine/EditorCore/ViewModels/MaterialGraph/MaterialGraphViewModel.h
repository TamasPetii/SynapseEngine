#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "MaterialGraphState.h"
#include "MaterialGraphIntent.h"
#include "EditorCore/Api/IMaterialApi.h"


namespace Syn {
    class MaterialGraphViewModel : public IViewModel<MaterialGraphState, MaterialGraphIntent> {
    public:
        MaterialGraphViewModel(IMaterialApi* materialApi);

        const MaterialGraphState& GetState() const override { return _state; }

        void SyncWithEngine() override;
        void Dispatch(const MaterialGraphIntent& intent) override;
    private:
        void BuildGraphFromEngine();
        void HandleCreateLink(const CreateLinkIntent& intent);
        void HandleDeleteLink(const DeleteLinkIntent& intent);
    private:
        IMaterialApi* _materialApi = nullptr;
        MaterialGraphState _state;
    };
}