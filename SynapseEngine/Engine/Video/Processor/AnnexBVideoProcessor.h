#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Processor/IVideoProcessor.h"

namespace Syn
{
    class SYN_API AnnexBVideoProcessor : public IVideoProcessor
    {
    public:
        AnnexBVideoProcessor() = default;
        ~AnnexBVideoProcessor() override = default;

        void Process(CookedVideoPacket& cookedPacket) override;
    };
}