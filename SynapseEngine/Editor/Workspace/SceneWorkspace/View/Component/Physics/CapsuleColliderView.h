#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Physics/CapsuleCollider/CapsuleColliderViewModel.h"

namespace Syn {
    class CapsuleColliderView : public IView<CapsuleColliderViewModel> {
    public:
        void Draw(CapsuleColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}