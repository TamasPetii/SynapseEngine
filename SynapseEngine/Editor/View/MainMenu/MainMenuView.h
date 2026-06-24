#pragma once
#include "../IView.h"
#include "EditorCore/ViewModels/MainMenu/MainMenuViewModel.h"
#include <imgui.h>

namespace Syn {
    class MainMenuView : public IView<MainMenuViewModel> {
    public:
        void Draw(MainMenuViewModel& vm) override;
    };
}