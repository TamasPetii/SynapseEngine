#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/ComponentViewModel.h"
#include <unordered_map>
#include <string>

#include "Core/TagView.h"
#include "Core/TransformView.h"
#include "Light/DirectionLightView.h"
#include "Light/PointLightView.h"

namespace Syn {
    class ComponentView : public IView<ComponentViewModel> {
    public:
        void Draw(ComponentViewModel& vm) override;
    private:
        TagView _tagView;
		TransformView _transformView;  
        DirectionLightView _directionLightView;
        PointLightView _pointLightView;
    };
}