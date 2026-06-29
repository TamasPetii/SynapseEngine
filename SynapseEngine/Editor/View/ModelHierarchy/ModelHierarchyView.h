#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/ModelHierarchy/ModelHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class ModelHierarchyView : public IView<ModelHierarchyViewModel> {
    public:
        void Draw(ModelHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(ModelHierarchyViewModel& vm);
        void RenderNodeRow(ModelHierarchyViewModel& vm, const ModelHierarchyNode& node);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}