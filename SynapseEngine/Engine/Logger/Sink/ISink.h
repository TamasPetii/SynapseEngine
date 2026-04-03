#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/LogMessage.h"
#include <mutex>

namespace Syn
{
    class SYN_API ISink {
    public:
        virtual ~ISink() = default;
        virtual void Log(const LogMessage& msg) = 0;
    protected:
        std::mutex _mtx;
    };
}

