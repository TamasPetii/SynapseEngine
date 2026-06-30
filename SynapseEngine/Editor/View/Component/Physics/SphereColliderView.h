#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Physics/SphereCollider/SphereColliderViewModel.h"

namespace Syn {
    class SphereColliderView : public IView<SphereColliderViewModel> {
    public:
        void Draw(SphereColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}