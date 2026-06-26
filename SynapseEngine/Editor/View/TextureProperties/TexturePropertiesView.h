#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/TextureProperties/TexturePropertiesViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class TexturePropertiesView : public IView<TexturePropertiesViewModel> {
    public:
        void Draw(TexturePropertiesViewModel& vm) override;
    private:
        void DrawReadOnlyProperty(const char* label, const char* value);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}