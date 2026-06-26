#pragma once
#include "EditorCore/Api/IPointLightApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class PointLightApiImpl : public IPointLightApi {
    public:
        PointLightApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasPointLight(EntityID entity) const override;
        glm::vec3 GetLightColor(EntityID entity) const override;
        float GetLightStrength(EntityID entity) const override;
        bool GetLightUseShadow(EntityID entity) const override;
        float GetLightRadius(EntityID entity) const override;
        float GetLightWeakenDistance(EntityID entity) const override;

        void SetLightColor(EntityID entity, const glm::vec3& color) override;
        void SetLightStrength(EntityID entity, float strength) override;
        void SetLightUseShadow(EntityID entity, bool useShadow) override;
        void SetLightRadius(EntityID entity, float radius) override;
        void SetLightWeakenDistance(EntityID entity, float distance) override;

        float GetShadowNearPlane(EntityID entity) const override;
        float GetShadowFarPlane(EntityID entity) const override;

        void SetShadowNearPlane(EntityID entity, float nearPlane) override;
        void SetShadowFarPlane(EntityID entity, float farPlane) override;
    private:
        SceneManager* _sceneManager;
    };
}