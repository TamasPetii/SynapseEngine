#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialProperties/MaterialPropertiesViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class MaterialPropertiesView : public IView<MaterialPropertiesViewModel> {
    public:
        void Draw(MaterialPropertiesViewModel& vm) override;
    private:
        void DrawTextureSlot(const char* label,
            uint32_t& currentTexId, const std::string& currentTexName, const std::vector<TextureOption>& texOptions,
            uint32_t& currentSampId, const std::string& currentSampName, const std::vector<SamplerOption>& sampOptions,
            bool& changed);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}