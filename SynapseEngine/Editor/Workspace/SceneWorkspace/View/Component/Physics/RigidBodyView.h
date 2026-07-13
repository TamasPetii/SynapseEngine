#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Physics/RigidBody/RigidBodyViewModel.h"

namespace Syn {
    class RigidBodyView : public IView<RigidBodyViewModel> {
    public:
        void Draw(RigidBodyViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}