#pragma once
#include "Engine/SynApi.h"
#include "Engine/Logger/ISink.h"
#include <fstream>

namespace Syn
{
    class SYN_API FileSink : public ISink {
    public:
        FileSink();
        void Log(const LogMessage& msg) override;
    private:
        std::ofstream _file;
    };
}




