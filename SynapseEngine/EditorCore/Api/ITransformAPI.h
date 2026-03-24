#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include <glm/glm.hpp>

namespace Syn {
    class ITransformAPI {
    public:
        virtual ~ITransformAPI() = default;

        virtual glm::vec3 GetEntityPosition(EntityID entity) const = 0;
        virtual glm::vec3 GetEntityRotation(EntityID entity) const = 0;
        virtual glm::vec3 GetEntityScale(EntityID entity) const = 0;

        virtual void SetEntityPosition(EntityID entity, const glm::vec3& position) = 0;
        virtual void SetEntityRotation(EntityID entity, const glm::vec3& rotation) = 0;
        virtual void SetEntityScale(EntityID entity, const glm::vec3& scale) = 0;
    };
}