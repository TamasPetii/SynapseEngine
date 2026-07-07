#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/MaterialViewport/MaterialViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class MaterialViewportView : public IView<MaterialViewportViewModel> {
    public:
        void Draw(MaterialViewportViewModel& vm) override;
    };
}