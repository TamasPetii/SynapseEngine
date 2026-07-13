#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Core/Transform/TransformViewModel.h"

namespace Syn {
    class TransformView : public IView<TransformViewModel> {
    public:
        void Draw(TransformViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}