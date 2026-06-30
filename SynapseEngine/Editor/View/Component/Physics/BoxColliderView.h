#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Physics/BoxCollider/BoxColliderViewModel.h"

namespace Syn {
    class BoxColliderView : public IView<BoxColliderViewModel> {
    public:
        void Draw(BoxColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}