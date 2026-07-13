#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Physics/BoxCollider/BoxColliderViewModel.h"

namespace Syn {
    class BoxColliderView : public IView<BoxColliderViewModel> {
    public:
        void Draw(BoxColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}