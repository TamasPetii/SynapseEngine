#pragma once
#include "IApi.h"
#include <glm/glm.hpp>
#include "EditorCore/Types/EntityHandle.h"
#include <cstdint>

namespace Syn {
    class IConvexColliderApi : public IApi {
    public:
        virtual ~IConvexColliderApi() = default;

        virtual bool HasConvexCollider(EntityID entity) const = 0;

        virtual uint32_t GetConvexColliderTargetLodLevel(EntityID entity) const = 0;
        virtual glm::vec3 GetConvexColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetConvexColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) = 0;
        virtual void SetConvexColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}