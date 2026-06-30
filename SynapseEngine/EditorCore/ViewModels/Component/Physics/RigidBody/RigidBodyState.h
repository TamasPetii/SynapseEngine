#pragma once
#include "Engine/Physics/PhysicsTypes.h"
#include <cstdint>

namespace Syn {
    struct RigidBodyState {
        bool hasComponent = false;

        PhysicsMotionType motionType;
        float mass;
        float friction;
        float restitution;
        uint32_t layer;
    };
}