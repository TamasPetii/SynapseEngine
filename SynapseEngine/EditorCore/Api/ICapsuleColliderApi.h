#pragma once
#include "IApi.h"
#include <glm/glm.hpp>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ICapsuleColliderApi : public IApi {
    public:
        virtual ~ICapsuleColliderApi() = default;

        virtual bool HasCapsuleCollider(EntityID entity) const = 0;

        virtual float GetCapsuleColliderRadius(EntityID entity) const = 0;
        virtual float GetCapsuleColliderHalfHeight(EntityID entity) const = 0;
        virtual glm::vec3 GetCapsuleColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetCapsuleColliderRadius(EntityID entity, float radius) = 0;
        virtual void SetCapsuleColliderHalfHeight(EntityID entity, float halfHeight) = 0;
        virtual void SetCapsuleColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}