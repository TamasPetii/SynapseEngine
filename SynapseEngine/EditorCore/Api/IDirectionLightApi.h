#pragma once
#include <glm/glm.hpp>
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class IDirectionLightApi {
    public:
        virtual ~IDirectionLightApi() = default;

        virtual bool HasDirectionLight(EntityID entity) const = 0;

        virtual glm::vec3 GetLightColor(EntityID entity) const = 0;
        virtual float GetLightStrength(EntityID entity) const = 0;
        virtual bool GetLightUseShadow(EntityID entity) const = 0;

        virtual void SetLightColor(EntityID entity, const glm::vec3& color) = 0;
        virtual void SetLightStrength(EntityID entity, float strength) = 0;
        virtual void SetLightUseShadow(EntityID entity, bool useShadow) = 0;

        virtual float GetShadowFarPlane(EntityID entity) const = 0;
        virtual glm::vec4 GetCascadeSplits(EntityID entity) const = 0;

        virtual void SetShadowFarPlane(EntityID entity, float farPlane) = 0;
        virtual void SetCascadeSplits(EntityID entity, const glm::vec4& splits) = 0;
    };
}