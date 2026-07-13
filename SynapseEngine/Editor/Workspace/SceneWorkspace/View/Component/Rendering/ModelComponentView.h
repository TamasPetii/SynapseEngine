#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Rendering/Model/ModelComponentViewModel.h"

namespace Syn {
    class ModelComponentView : public IView<ModelComponentViewModel> {
    public:
        void Draw(ModelComponentViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}