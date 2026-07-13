#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Hierarchy/HierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class HierarchyView : public IView<HierarchyViewModel> {
    public:
        void Draw(HierarchyViewModel& vm) override;
    private:
        void RenderTopBar(HierarchyViewModel& vm);
        void RenderEntityRow(HierarchyViewModel& vm, const HierarchyNode& node);
        void HandleDragAndDrop(HierarchyViewModel& vm, EntityID entity);
        void RenderContextMenu(HierarchyViewModel& vm, EntityID contextEntity);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}