#pragma once
#include <glm/glm.hpp>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class IPointLightApi {
    public:
        virtual ~IPointLightApi() = default;

        virtual bool HasPointLight(EntityID entity) const = 0;

        virtual glm::vec3 GetLightColor(EntityID entity) const = 0;
        virtual float GetLightStrength(EntityID entity) const = 0;
        virtual bool GetLightUseShadow(EntityID entity) const = 0;
        virtual float GetLightRadius(EntityID entity) const = 0;
        virtual float GetLightWeakenDistance(EntityID entity) const = 0;

        virtual void SetLightColor(EntityID entity, const glm::vec3& color) = 0;
        virtual void SetLightStrength(EntityID entity, float strength) = 0;
        virtual void SetLightUseShadow(EntityID entity, bool useShadow) = 0;
        virtual void SetLightRadius(EntityID entity, float radius) = 0;
        virtual void SetLightWeakenDistance(EntityID entity, float distance) = 0;

        virtual float GetShadowNearPlane(EntityID entity) const = 0;
        virtual float GetShadowFarPlane(EntityID entity) const = 0;

        virtual void SetShadowNearPlane(EntityID entity, float nearPlane) = 0;
        virtual void SetShadowFarPlane(EntityID entity, float farPlane) = 0;
    };
}