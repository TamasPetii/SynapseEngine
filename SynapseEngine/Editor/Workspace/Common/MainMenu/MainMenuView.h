#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/Common/MainMenu/MainMenuViewModel.h"
#include <imgui.h>

namespace Syn {
    class MainMenuView : public IView<MainMenuViewModel> {
    public:
        void Draw(MainMenuViewModel& vm) override;
    };
}