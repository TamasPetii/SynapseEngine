#pragma once
#include "Engine/Component/Components.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"

namespace Syn
{
    using FullSceneSnapshot = SceneSnapshot<
        TransformComponent,
        CameraComponent,
        ModelComponent,
        TagComponent,
        AnimationComponent,
        BoxColliderComponent,
        SphereColliderComponent,
        CapsuleColliderComponent,
		ConvexColliderComponent,
        MeshColliderComponent,
        RigidBodyComponent,
		DirectionLightComponent,
		DirectionLightShadowComponent,
		PointLightComponent,
		PointLightShadowComponent,
		SpotLightComponent,
		SpotLightShadowComponent,
		MaterialOverrideComponent
    >;
}