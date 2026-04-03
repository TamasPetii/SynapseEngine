#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
    using PhysicsBodyID = uint32_t;
    constexpr PhysicsBodyID INVALID_BODY_ID = 0xFFFFFFFF;

    struct SYN_API PhysicsInitParams
    {
        uint32_t maxBodies = 100000;
        uint32_t numBodyMutexes = 0;
        uint32_t maxBodyPairs = 100000;
        uint32_t maxContactConstraints = 100000;
        uint32_t tempAllocatorSizeMB = 25;
    };

    enum class SYN_API PhysicsMotionType
    {
        Static,
        Kinematic,
        Dynamic
    };

    struct SYN_API PhysicsBodySettings
    {
        PhysicsMotionType motionType = PhysicsMotionType::Dynamic;
        uint16_t layer = 1;

        float mass = 1.0f;
        float friction = 0.2f;
        float restitution = 0.0f;

        bool isSensor = false;
    };
}