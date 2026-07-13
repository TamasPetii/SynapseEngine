#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITagApi.h"
#include "TagState.h"
#include "TagIntent.h"

namespace Syn {
    class TagViewModel : public IViewModel<TagState, TagIntent> {
    public:
        TagViewModel(ISelectionApi* selectionApi, ITagApi* tagApi);
        ~TagViewModel() override = default;

        const TagState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const TagIntent& intent) override;

    private:
        ISelectionApi* _selectionApi = nullptr;
        ITagApi* _tagApi = nullptr;
        TagState _state;
    };
}