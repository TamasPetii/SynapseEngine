#pragma once
#include "Engine/SynApi.h"
#include "Engine/Animation/Data/Cooked/CookedAnimation.h"
#include "../IAnimationProcessor.h"

#include <chrono>
#include <taskflow/taskflow.hpp>

namespace Syn
{
    class SYN_API AnimationColliderProcessor : public IAnimationProcessor
    {
    public:
        virtual void Process(CookedAnimation& inOutAnimation, const CpuModelData& baseModel) override;
    private:
        void ComputeFrameColliders(uint32_t frameIndex, CookedAnimation& anim, const CpuModelData& model, tf::Subflow& subflow);
        void ComputeGlobalAnimationCollider(CookedAnimation& anim);
    };
}