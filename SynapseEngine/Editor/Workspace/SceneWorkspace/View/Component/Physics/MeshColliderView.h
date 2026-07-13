#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Physics/MeshCollider/MeshColliderViewModel.h"

namespace Syn {
    class MeshColliderView : public IView<MeshColliderViewModel> {
    public:
        void Draw(MeshColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}