#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "../IAnimationProcessor.h"
#include <taskflow/taskflow.hpp>

namespace Syn
{
    class SYN_API AnimationColliderProcessor : public IAnimationProcessor
    {
    public:
        virtual void Process(CookedAnimation& inOutAnimation, const CookedModel& baseModel) override;
    private:
        void ComputeFrameColliders(uint32_t frameIndex, CookedAnimation& anim, const CookedModel& model, tf::Subflow& subflow);
    };
}