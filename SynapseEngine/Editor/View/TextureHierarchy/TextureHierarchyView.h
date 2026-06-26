#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/TextureHierarchy/TextureHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class TextureHierarchyView : public IView<TextureHierarchyViewModel> {
    public:
        void Draw(TextureHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(TextureHierarchyViewModel& vm);
        void RenderTextureRow(TextureHierarchyViewModel& vm, const TextureNode& node);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}