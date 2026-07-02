#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Rendering/MaterialOverride/MaterialOverrideViewModel.h"

namespace Syn {
    class MaterialOverrideView : public IView<MaterialOverrideViewModel> {
    public:
        void Draw(MaterialOverrideViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}