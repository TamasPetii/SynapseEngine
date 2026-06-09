#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Light/DirectionLight/DirectionLightViewModel.h"

namespace Syn {
    class DirectionLightView : public IView<DirectionLightViewModel> {
    public:
        void Draw(DirectionLightViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}