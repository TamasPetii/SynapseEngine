#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/ComponentViewModel.h"
#include <unordered_map>
#include <string>

#include "Core/TagView.h"
#include "Core/TransformView.h"
#include "Core/CameraView.h"
#include "Light/DirectionLightView.h"
#include "Light/PointLightView.h"
#include "Light/SpotLightView.h"

#include "Physics/BoxColliderView.h"
#include "Physics/SphereColliderView.h"
#include "Physics/CapsuleColliderView.h"
#include "Physics/ConvexColliderView.h"
#include "Physics/MeshColliderView.h"
#include "Physics/RigidBodyView.h"

#include "Rendering/ModelComponentView.h"
#include "Rendering/AnimationView.h"

namespace Syn {
    class ComponentView : public IView<ComponentViewModel> {
    public:
        void Draw(ComponentViewModel& vm) override;
    private:
        TagView _tagView;
        CameraView _cameraView;
		TransformView _transformView;  
        DirectionLightView _directionLightView;
        PointLightView _pointLightView;
        SpotLightView _spotLightView;
        BoxColliderView _boxColliderView;
        SphereColliderView _sphereColliderView;
        CapsuleColliderView _capsuleColliderView;
        ConvexColliderView _convexColliderView;
        MeshColliderView _meshColliderView;
        RigidBodyView _rigidBodyView;
		ModelComponentView _modelComponentView;
		AnimationView _animationView;
    };
}