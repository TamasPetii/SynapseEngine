#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "PhysicsTypes.h"

namespace Syn
{
    class SYN_API IPhysicsEngine
    {
    public:
        virtual ~IPhysicsEngine() = default;

        virtual void Init(const PhysicsInitParams& params = {}) = 0;
        virtual void Shutdown() = 0;
        virtual void Update(float deltaTime) = 0;

        virtual void SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation) = 0;
        virtual void GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const = 0;
        virtual bool IsBodyActive(PhysicsBodyID bodyId) const = 0;
    };
}