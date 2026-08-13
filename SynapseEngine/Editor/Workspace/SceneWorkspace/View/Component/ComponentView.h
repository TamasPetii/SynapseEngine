// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/ComponentViewModel.h"
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
#include "Rendering/MaterialOverrideView.h"
#include "Rendering/PipelineOverrideView.h"
#include "Audio/AudioSourceView.h"
#include "Audio/AudioListenerView.h"

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
        MaterialOverrideView _materialOverrideView;
        PipelineOverrideView _pipelineOverrideView;
		AudioSourceView _audioSourceView;
        AudioListenerView _audioListenerView;
    };
}