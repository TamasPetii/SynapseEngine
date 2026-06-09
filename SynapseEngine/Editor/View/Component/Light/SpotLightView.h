#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Light/SpotLight/SpotLightViewModel.h"

namespace Syn {
    class SpotLightView : public IView<SpotLightViewModel> {
    public:
        void Draw(SpotLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}