#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Light/PointLight/PointLightViewModel.h"

namespace Syn {
    class PointLightView : public IView<PointLightViewModel> {
    public:
        void Draw(PointLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}