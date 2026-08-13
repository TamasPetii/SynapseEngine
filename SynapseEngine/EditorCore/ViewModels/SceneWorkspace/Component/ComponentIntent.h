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
#include "Core/Tag/TagIntent.h"
#include "Core/Camera/CameraIntent.h"
#include "Core/Transform/TransformIntent.h"
#include "Light/DirectionLight/DirectionLightIntent.h"
#include "Light/PointLight/PointLightIntent.h"
#include "Light/SpotLight/SpotLightIntent.h"
#include "Rendering/Model/ModelComponentIntent.h"
#include "Rendering/Animation/AnimationIntent.h"
#include "Rendering/MaterialOverride/MaterialOverrideIntent.h"
#include "Rendering/PipelineOverride/PipelineOverrideIntent.h"
#include "Physics/BoxCollider/BoxColliderIntent.h"
#include "Physics/SphereCollider/SphereColliderIntent.h"
#include "Physics/CapsuleCollider/CapsuleColliderIntent.h"
#include "Physics/ConvexCollider/ConvexColliderIntent.h"
#include "Physics/MeshCollider/MeshColliderIntent.h"
#include "Physics/RigidBody/RigidBodyIntent.h"
#include "Audio/AudioSource/AudioSourceIntent.h"
#include "Audio/AudioListener/AudioListenerIntent.h"
#include <variant>

namespace Syn {
    using ComponentIntent = std::variant<
        TagIntent,
        CameraIntent,
        TransformIntent,
        DirectionLightIntent,
        PointLightIntent,
        SpotLightIntent,
        ModelComponentIntent,
		AnimationIntent,
        BoxColliderIntent,
        SphereColliderIntent,
        CapsuleColliderIntent,
        ConvexColliderIntent,
        MeshColliderIntent,
        RigidBodyIntent,
        MaterialOverrideIntent,
        PipelineOverrideIntent, 
        AudioSourceIntent,
        AudioListenerIntent
    >;
}