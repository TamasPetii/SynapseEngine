#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/ISink.h"

namespace Syn
{
    class SYN_API MemorySink : public ISink {
    public:
        explicit MemorySink(size_t maxHistory = 1000);
        void Log(const LogMessage& msg) override;
        void Clear();
		const std::vector<LogMessage>& GetLogs() const { return _logs; }
    private:
        std::vector<LogMessage> _logs;
        size_t _maxHistory;
    };
}
