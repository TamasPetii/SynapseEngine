#include "MemorySink.h"

namespace Syn
{
    MemorySink::MemorySink(size_t maxHistory) : 
		_maxHistory(maxHistory)
    {
    }

    void MemorySink::Log(const LogMessage& msg) {
        std::scoped_lock lock(_mtx);

        _logs.push_back(msg);

        if (_logs.size() > _maxHistory) {
            _logs.erase(_logs.begin());
        }
    }

    void MemorySink::Clear() {
        std::scoped_lock lock(_mtx);

        _logs.clear();
    }
}