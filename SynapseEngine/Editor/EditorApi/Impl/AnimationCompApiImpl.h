#pragma once
#include "EditorCore/Api/IAnimationCompApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class AnimationCompApiImpl : public IAnimationCompApi {
    public:
        AnimationCompApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasAnimation(EntityID entity) const override;
        float GetAnimationSpeed(EntityID entity) const override;
        uint32_t GetAnimationIndex(EntityID entity) const override;

        void SetAnimationSpeed(EntityID entity, float speed) override;
        void SetAnimationIndex(EntityID entity, uint32_t index) override;

        std::vector<std::pair<uint32_t, std::string>> GetAvailableAnimations() const override;
    private:
        SceneManager* _sceneManager;
    };
}