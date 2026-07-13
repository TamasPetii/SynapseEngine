#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "TextureHierarchyState.h"
#include "TextureHierarchyIntent.h"
#include "EditorCore/Api/ITextureApi.h"

namespace Syn {
    class TextureHierarchyViewModel : public IViewModel<TextureHierarchyState, TextureHierarchyIntent> {
    public:
        TextureHierarchyViewModel(ITextureApi* textureApi);
        ~TextureHierarchyViewModel() override = default;

        const TextureHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const TextureHierarchyIntent& intent) override;

    private:
        void RebuildList();
    private:
        ITextureApi* _textureApi = nullptr;
        TextureHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
    };
}