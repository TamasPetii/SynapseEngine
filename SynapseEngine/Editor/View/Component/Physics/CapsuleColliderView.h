#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Physics/CapsuleCollider/CapsuleColliderViewModel.h"

namespace Syn {
    class CapsuleColliderView : public IView<CapsuleColliderViewModel> {
    public:
        void Draw(CapsuleColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}