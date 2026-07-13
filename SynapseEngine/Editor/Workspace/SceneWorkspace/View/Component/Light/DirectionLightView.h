#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Light/DirectionLight/DirectionLightViewModel.h"

namespace Syn {
    class DirectionLightView : public IView<DirectionLightViewModel> {
    public:
        void Draw(DirectionLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}