#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Hierarchy/HierarchyViewModel.h"

namespace Syn {
    class HierarchyView : public IView<HierarchyViewModel> {
    public:
        void Draw(HierarchyViewModel& vm) override;
    private:
        void RenderTopBar(HierarchyViewModel& vm);
        void RenderEntityRow(HierarchyViewModel& vm, const HierarchyNode& node);
        void HandleDragAndDrop(HierarchyViewModel& vm, EntityID entity);
        void RenderContextMenu(HierarchyViewModel& vm, EntityID contextEntity);
    };
}