#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/MaterialHierarchy/MaterialHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class MaterialHierarchyView : public IView<MaterialHierarchyViewModel> {
    public:
        void Draw(MaterialHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(MaterialHierarchyViewModel& vm);
        void RenderMaterialRow(MaterialHierarchyViewModel& vm, const MaterialNode& node);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}