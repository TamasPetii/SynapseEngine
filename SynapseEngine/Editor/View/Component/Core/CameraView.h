#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Core/Camera/CameraViewModel.h"

namespace Syn {
    class CameraView : public IView<CameraViewModel> {
    public:
        void Draw(CameraViewModel& vm) override;
    private:
        bool _isCardOpen = true;
    };
}