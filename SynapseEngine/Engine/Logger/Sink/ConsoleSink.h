#pragma once
#include "Engine/SynApi.h"
#include "ISink.h"

namespace Syn
{
    class SYN_API ConsoleSink : public ISink {
    public:
        void Log(const LogMessage& msg) override;
    };
}
