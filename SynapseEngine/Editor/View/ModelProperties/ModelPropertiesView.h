#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/ModelProperties/ModelPropertiesViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class ModelPropertiesView : public IView<ModelPropertiesViewModel> {
    public:
        void Draw(ModelPropertiesViewModel& vm) override;
    private:
        void DrawPropertyRow(const char* label, const std::string& value);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}