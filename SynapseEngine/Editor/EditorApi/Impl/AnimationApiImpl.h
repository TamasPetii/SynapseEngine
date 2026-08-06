#pragma once
#include "EditorCore/Api/IAnimationApi.h"
#include "Engine/Animation/AnimationManager.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class AnimationApiImpl : public IAnimationApi {
    public:
        AnimationApiImpl(AnimationManager* animManager, SceneManager* sceneManager)
            : _animManager(animManager), _sceneManager(sceneManager) {}

        std::vector<AnimationItemData> GetAllAnimations() const override;
        uint64_t GetVersion() const override;

        void SetSelected(uint32_t animationId) override;
        uint32_t GetSelected() const override;

        const CpuAnimationData* GetAnimationCpuData(uint32_t animationId) const override;

        void ApplyAnimationToPreviewObject(uint32_t animationId) override;
    private:
        AnimationManager* _animManager;
        SceneManager* _sceneManager;
        uint32_t _selectedAnimationId = INVALID_ANIMATION_ID;
    };
}