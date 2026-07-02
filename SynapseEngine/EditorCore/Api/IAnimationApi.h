#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    class IAnimationApi {
    public:
        virtual ~IAnimationApi() = default;

        virtual bool HasAnimation(EntityID entity) const = 0;

        virtual float GetAnimationSpeed(EntityID entity) const = 0;
        virtual uint32_t GetAnimationIndex(EntityID entity) const = 0;

        virtual void SetAnimationSpeed(EntityID entity, float speed) = 0;
        virtual void SetAnimationIndex(EntityID entity, uint32_t index) = 0;

        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailableAnimations() const = 0;
    };
}