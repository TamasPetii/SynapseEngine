#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Rendering/Model/ModelComponentViewModel.h"

namespace Syn {
    class ModelComponentView : public IView<ModelComponentViewModel> {
    public:
        void Draw(ModelComponentViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}