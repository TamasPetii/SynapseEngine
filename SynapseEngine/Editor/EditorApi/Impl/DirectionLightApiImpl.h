#pragma once
#include "EditorCore/Api/IDirectionLightApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class DirectionLightApiImpl : public IDirectionLightApi {
    public:
        DirectionLightApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasDirectionLight(EntityID entity) const override;
        glm::vec3 GetLightColor(EntityID entity) const override;
        float GetLightStrength(EntityID entity) const override;
        bool GetLightUseShadow(EntityID entity) const override;

        void SetLightColor(EntityID entity, const glm::vec3& color) override;
        void SetLightStrength(EntityID entity, float strength) override;
        void SetLightUseShadow(EntityID entity, bool useShadow) override;

        float GetShadowFarPlane(EntityID entity) const override;
        glm::vec4 GetCascadeSplits(EntityID entity) const override;
        void SetShadowFarPlane(EntityID entity, float farPlane) override;
        void SetCascadeSplits(EntityID entity, const glm::vec4& splits) override;
    private:
        SceneManager* _sceneManager;
    };
}