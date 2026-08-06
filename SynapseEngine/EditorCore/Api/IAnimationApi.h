#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "IApi.h"
#include "Engine/Animation/Data/Cpu/CpuAnimationData.h"

namespace Syn
{
    constexpr uint32_t INVALID_ANIMATION_ID = 0xFFFFFFFF;

    struct AnimationItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IAnimationApi : public IApi {
    public:
        virtual ~IAnimationApi() = default;

        virtual std::vector<AnimationItemData> GetAllAnimations() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t animationId) = 0;
        virtual uint32_t GetSelected() const = 0;

        virtual const CpuAnimationData* GetAnimationCpuData(uint32_t animationId) const = 0;

        virtual void ApplyAnimationToPreviewObject(uint32_t animationId) = 0;
    };
}