#pragma once
#include "Engine/Component/Components.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"

namespace Syn
{
    using FullSceneSnapshot = SceneSnapshot<
        TransformComponent,
        CameraComponent,
        ModelComponent,
        AnimationComponent,
        BoxColliderComponent,
        SphereColliderComponent,
        CapsuleColliderComponent,
        RigidBodyComponent
    >;
}