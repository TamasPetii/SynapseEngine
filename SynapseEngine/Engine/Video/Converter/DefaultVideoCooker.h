#pragma once
#include "Engine/SynApi.h"
#include "IVideoCooker.h"

namespace Syn
{
    class SYN_API DefaultVideoCooker : public IVideoCooker
    {
    public:
        DefaultVideoCooker() = default;
        ~DefaultVideoCooker() override = default;

        CookedVideoPacket Cook(const RawVideoPacket& rawPacket) override;
    };
}