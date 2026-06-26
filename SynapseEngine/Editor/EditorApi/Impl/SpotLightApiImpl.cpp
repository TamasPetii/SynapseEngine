#include "SpotLightApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"

namespace Syn {
    bool SpotLightApiImpl::HasSpotLight(EntityID entity) const {
        return EditorApiUtils::HasComponent<SpotLightComponent>(_sceneManager, entity);
    }
    glm::vec3 SpotLightApiImpl::GetLightColor(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.color; }, glm::vec3(1.0f));
    }
    float SpotLightApiImpl::GetLightStrength(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.strength; }, 1.0f);
    }
    bool SpotLightApiImpl::GetLightUseShadow(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.useShadow; }, false);
    }
    float SpotLightApiImpl::GetLightRange(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.range; }, 10.0f);
    }
    float SpotLightApiImpl::GetLightWeakenDistance(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.weakenDistance; }, 0.0f);
    }
    float SpotLightApiImpl::GetLightInnerAngle(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.innerAngle; }, 12.5f);
    }
    float SpotLightApiImpl::GetLightOuterAngle(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightComponent>(_sceneManager, entity, [](const auto& c) { return c.outerAngle; }, 17.5f);
    }

    void SpotLightApiImpl::SetLightColor(EntityID entity, const glm::vec3& color) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.color = color; });
    }
    void SpotLightApiImpl::SetLightStrength(EntityID entity, float strength) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.strength = strength; });
    }
    void SpotLightApiImpl::SetLightRange(EntityID entity, float range) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.range = range; });
    }
    void SpotLightApiImpl::SetLightWeakenDistance(EntityID entity, float dist) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.weakenDistance = dist; });
    }
    void SpotLightApiImpl::SetLightInnerAngle(EntityID entity, float angle) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.innerAngle = angle; });
    }
    void SpotLightApiImpl::SetLightOuterAngle(EntityID entity, float angle) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.outerAngle = angle; });
    }
    void SpotLightApiImpl::SetLightUseShadow(EntityID entity, bool useShadow) {
        EditorApiUtils::ModifyComponent<SpotLightComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            if (c.useShadow != useShadow) {
                c.useShadow = useShadow;
                pool->template SetBit<SHADOW_TOGGLE_BIT>(entity);
            }
        });
    }

    float SpotLightApiImpl::GetShadowNearPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.nearPlane; }, 0.1f);
    }
    float SpotLightApiImpl::GetShadowFarPlane(EntityID entity) const {
        return EditorApiUtils::ReadComponent<SpotLightShadowComponent>(_sceneManager, entity, [](const auto& c) { return c.farPlane; }, 10.0f);
    }
    void SpotLightApiImpl::SetShadowNearPlane(EntityID entity, float nearPlane) {
        EditorApiUtils::ModifyComponent<SpotLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.nearPlane = nearPlane; });
    }
    void SpotLightApiImpl::SetShadowFarPlane(EntityID entity, float farPlane) {
        EditorApiUtils::ModifyComponent<SpotLightShadowComponent>(_sceneManager, entity, [&](auto& c, auto pool) { c.farPlane = farPlane; });
    }
}