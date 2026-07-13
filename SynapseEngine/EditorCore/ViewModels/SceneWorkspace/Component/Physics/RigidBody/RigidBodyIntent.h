#pragma once
#include "Engine/Physics/PhysicsTypes.h"
#include <variant>
#include <cstdint>

namespace Syn
{
    struct SetRigidBodyMotionTypeIntent {
        PhysicsMotionType motionType;
    };

    struct SetRigidBodyMassIntent {
        float mass;
        bool isDragging;
    };

    struct SetRigidBodyFrictionIntent {
        float friction;
        bool isDragging;
    };

    struct SetRigidBodyRestitutionIntent {
        float restitution;
        bool isDragging;
    };

    struct SetRigidBodyLayerIntent {
        uint32_t layer;
    };

    using RigidBodyIntent = std::variant<
        SetRigidBodyMotionTypeIntent,
        SetRigidBodyMassIntent,
        SetRigidBodyFrictionIntent,
        SetRigidBodyRestitutionIntent,
        SetRigidBodyLayerIntent>;
}