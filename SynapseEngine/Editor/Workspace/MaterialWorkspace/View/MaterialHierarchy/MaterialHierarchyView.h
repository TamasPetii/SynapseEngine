#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialHierarchy/MaterialHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class MaterialHierarchyView : public IView<MaterialHierarchyViewModel> {
    public:
        void Draw(MaterialHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(MaterialHierarchyViewModel& vm);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}