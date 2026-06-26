#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/MaterialProperties/MaterialPropertiesViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class MaterialPropertiesView : public IView<MaterialPropertiesViewModel> {
    public:
        void Draw(MaterialPropertiesViewModel& vm) override;
    private:
        void DrawTextureSlot(const char* label, uint32_t& currentTexId, const std::string& currentName, const std::vector<TextureOption>& options, bool& changed);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}