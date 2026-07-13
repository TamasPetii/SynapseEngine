#pragma once
#include <glm/glm.hpp>
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ISphereColliderApi : public IApi {
    public:
        virtual ~ISphereColliderApi() = default;

        virtual bool HasSphereCollider(EntityID entity) const = 0;

        virtual float GetSphereColliderRadius(EntityID entity) const = 0;
        virtual glm::vec3 GetSphereColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetSphereColliderRadius(EntityID entity, float radius) = 0;
        virtual void SetSphereColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}