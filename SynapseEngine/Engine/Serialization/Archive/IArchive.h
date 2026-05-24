#pragma once
#include "Engine/SynApi.h"
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API IArchive {
    public:
        static std::vector<std::string> GetSupportedExtensions() { return {}; }
    };


}