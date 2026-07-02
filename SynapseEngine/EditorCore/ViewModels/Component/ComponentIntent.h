#pragma once
#include "Core/Tag/TagIntent.h"
#include "Core/Camera/CameraIntent.h"
#include "Core/Transform/TransformIntent.h"
#include "Light/DirectionLight/DirectionLightIntent.h"
#include "Light/PointLight/PointLightIntent.h"
#include "Light/SpotLight/SpotLightIntent.h"
#include "Rendering/Model/ModelComponentIntent.h"
#include "Rendering/Animation/AnimationIntent.h"
#include "Physics/BoxCollider/BoxColliderIntent.h"
#include "Physics/SphereCollider/SphereColliderIntent.h"
#include "Physics/CapsuleCollider/CapsuleColliderIntent.h"
#include "Physics/ConvexCollider/ConvexColliderIntent.h"
#include "Physics/MeshCollider/MeshColliderIntent.h"
#include "Physics/RigidBody/RigidBodyIntent.h"
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
        RigidBodyIntent
    >;
}