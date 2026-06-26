#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Core/Transform/TransformViewModel.h"

namespace Syn {
    class TransformView : public IView<TransformViewModel> {
    public:
        void Draw(TransformViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}