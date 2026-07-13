#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialViewport/MaterialViewportViewModel.h"
#include <imgui.h>

namespace Syn {
    class MaterialViewportView : public IView<MaterialViewportViewModel> {
    public:
        void Draw(MaterialViewportViewModel& vm) override;
    };
}