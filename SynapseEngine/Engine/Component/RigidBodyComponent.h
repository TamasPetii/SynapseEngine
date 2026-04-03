#pragma once
#include "BaseComponent/Component.h"
#include "Engine/Physics/PhysicsTypes.h"

namespace Syn
{
    struct SYN_API RigidBodyComponent : public Component
    {
        PhysicsMotionType motionType = PhysicsMotionType::Dynamic;

        float mass = 1.0f;
        float friction = 0.2f;
        float restitution = 0.0f;
        uint16_t layer = 1;

        PhysicsBodyID bodyID = INVALID_BODY_ID;
    };
}