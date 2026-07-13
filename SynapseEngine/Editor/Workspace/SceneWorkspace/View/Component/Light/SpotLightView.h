#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Light/SpotLight/SpotLightViewModel.h"

namespace Syn {
    class SpotLightView : public IView<SpotLightViewModel> {
    public:
        void Draw(SpotLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}