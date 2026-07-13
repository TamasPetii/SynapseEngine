#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/TextureWorkspace/TextureProperties/TexturePropertiesViewModel.h"
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