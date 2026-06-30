#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Physics/RigidBody/RigidBodyViewModel.h"

namespace Syn {
    class RigidBodyView : public IView<RigidBodyViewModel> {
    public:
        void Draw(RigidBodyViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}