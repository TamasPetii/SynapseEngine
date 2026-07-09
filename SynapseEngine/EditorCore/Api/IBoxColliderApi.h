#pragma once
#include <glm/glm.hpp>
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class IBoxColliderApi : public IApi {
    public:
        virtual ~IBoxColliderApi() = default;

        virtual bool HasBoxCollider(EntityID entity) const = 0;

        virtual glm::vec3 GetBoxColliderHalfExtents(EntityID entity) const = 0;
        virtual glm::vec3 GetBoxColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetBoxColliderHalfExtents(EntityID entity, const glm::vec3& halfExtents) = 0;
        virtual void SetBoxColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}