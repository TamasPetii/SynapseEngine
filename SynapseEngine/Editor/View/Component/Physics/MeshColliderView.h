#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Physics/MeshCollider/MeshColliderViewModel.h"

namespace Syn {
    class MeshColliderView : public IView<MeshColliderViewModel> {
    public:
        void Draw(MeshColliderViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}