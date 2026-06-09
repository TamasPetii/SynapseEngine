#include "TransformApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn {
    glm::vec3 TransformApiImpl::GetEntityPosition(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.translation; }, glm::vec3(0.0f));
    }

    void TransformApiImpl::SetEntityPosition(EntityID entity, const glm::vec3& position) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.translation = position; 
            pool->template SetBit<TRANSFORM_POS_CHANGED>(entity); 
        });
    }

    glm::vec3 TransformApiImpl::GetEntityRotation(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.rotation; }, glm::vec3(0.0f));
    }

    void TransformApiImpl::SetEntityRotation(EntityID entity, const glm::vec3& rotation) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.rotation = rotation; 
            pool->template SetBit<TRANSFORM_ROT_CHANGED>(entity); 
        });
    }

    glm::vec3 TransformApiImpl::GetEntityScale(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.scale; }, glm::vec3(1.0f));
    }

    void TransformApiImpl::SetEntityScale(EntityID entity, const glm::vec3& scale) {
        EditorApiUtils::ModifyComponent<TransformComponent>(_sceneManager, entity, [&](auto& c, auto pool) { 
            c.scale = scale; 
            pool->template SetBit<TRANSFORM_SCALE_CHANGED>(entity); 
        });
    }

    glm::mat4 TransformApiImpl::GetEntityWorldMatrix(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TransformComponent>(_sceneManager, entity, [](const auto& c) { return c.transform; }, glm::mat4(1.0f));
    }
}