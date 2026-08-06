#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/AnimationWorkspace/AnimationHierarchy/AnimationHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class AnimationHierarchyView : public IView<AnimationHierarchyViewModel> {
    public:
        void Draw(AnimationHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(AnimationHierarchyViewModel& vm);
        void RenderNodeRow(AnimationHierarchyViewModel& vm, const AnimationHierarchyNode& node);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}