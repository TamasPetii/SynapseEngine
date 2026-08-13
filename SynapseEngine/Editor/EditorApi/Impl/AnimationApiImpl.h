// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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