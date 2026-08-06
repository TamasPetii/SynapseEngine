#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/AudioWorkspace/AudioHierarchy/AudioHierarchyViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class AudioHierarchyView : public IView<AudioHierarchyViewModel> {
    public:
        void Draw(AudioHierarchyViewModel& vm) override;
    private:
        void RenderTopBar(AudioHierarchyViewModel& vm);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}