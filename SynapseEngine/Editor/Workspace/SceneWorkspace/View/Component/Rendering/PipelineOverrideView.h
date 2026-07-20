#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Rendering/PipelineOverride/PipelineOverrideViewModel.h"

namespace Syn {
    class PipelineOverrideView : public IView<PipelineOverrideViewModel> {
    public:
        void Draw(PipelineOverrideViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}