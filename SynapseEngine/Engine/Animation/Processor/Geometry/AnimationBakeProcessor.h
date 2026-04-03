#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Processor/IAnimationProcessor.h"

namespace Syn
{
    class SYN_API AnimationBakeProcessor : public IAnimationProcessor
    {
    public:
        virtual void Process(CookedAnimation& inOutAnimation, const CookedModel& baseModel) override;
    private:
        void BakeFrame(uint32_t frameIndex, CookedAnimation& anim);

        glm::vec3 InterpolatePosition(const BoneTrack& track, float time);
        glm::quat InterpolateRotation(const BoneTrack& track, float time);
        glm::vec3 InterpolateScale(const BoneTrack& track, float time);
    };
}