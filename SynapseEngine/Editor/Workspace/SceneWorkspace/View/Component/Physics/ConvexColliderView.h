#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Physics/ConvexCollider/ConvexColliderViewModel.h"

namespace Syn {
    class ConvexColliderView : public IView<ConvexColliderViewModel> {
    public:
        void Draw(ConvexColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}