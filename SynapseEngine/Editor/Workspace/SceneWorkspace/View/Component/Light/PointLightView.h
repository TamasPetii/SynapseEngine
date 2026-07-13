#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Light/PointLight/PointLightViewModel.h"

namespace Syn {
    class PointLightView : public IView<PointLightViewModel> {
    public:
        void Draw(PointLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}