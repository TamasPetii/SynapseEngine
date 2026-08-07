#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Processor/IVideoProcessor.h"

namespace Syn
{
    class SYN_API VulkanAnnexBVideoProcessor : public IVideoProcessor
    {
    public:
        VulkanAnnexBVideoProcessor() = default;
        ~VulkanAnnexBVideoProcessor() override = default;

        void Process(CookedVideoPacket& cookedPacket) override;
    };
}