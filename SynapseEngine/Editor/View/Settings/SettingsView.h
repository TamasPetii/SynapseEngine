#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Settings/SettingsViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class SettingsView : public IView<SettingsViewModel> {
    public:
        void Draw(SettingsViewModel& vm) override;
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}