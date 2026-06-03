#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/API/ISelectionAPI.h"
#include "EditorCore/API/ITagAPI.h"
#include "TagState.h"
#include "TagIntent.h"

namespace Syn {
    class TagViewModel : public IViewModel<TagState, TagIntent> {
    public:
        TagViewModel(ISelectionAPI* selectionApi, ITagAPI* tagApi);
        ~TagViewModel() override = default;

        const TagState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const TagIntent& intent) override;

    private:
        ISelectionAPI* _selectionApi = nullptr;
        ITagAPI* _tagApi = nullptr;
        TagState _state;
    };
}