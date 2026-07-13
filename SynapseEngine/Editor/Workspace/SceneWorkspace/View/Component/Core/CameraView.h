#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Core/Camera/CameraViewModel.h"

namespace Syn {
    class CameraView : public IView<CameraViewModel> {
    public:
        void Draw(CameraViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}