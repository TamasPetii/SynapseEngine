#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/ISink.h"

namespace Syn
{
    class SYN_API ConsoleSink : public ISink {
    public:
        void Log(const LogMessage& msg) override;
    };
}
