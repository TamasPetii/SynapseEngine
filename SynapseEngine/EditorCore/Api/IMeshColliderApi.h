#pragma once
#include <glm/glm.hpp>
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"
#include <cstdint>

namespace Syn {
    class IMeshColliderApi : public IApi {
    public:
        virtual ~IMeshColliderApi() = default;

        virtual bool HasMeshCollider(EntityID entity) const = 0;

        virtual uint32_t GetMeshColliderTargetLodLevel(EntityID entity) const = 0;
        virtual glm::vec3 GetMeshColliderLocalOffset(EntityID entity) const = 0;

        virtual void SetMeshColliderTargetLodLevel(EntityID entity, uint32_t targetLodLevel) = 0;
        virtual void SetMeshColliderLocalOffset(EntityID entity, const glm::vec3& localOffset) = 0;
    };
}