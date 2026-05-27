#pragma once
#include "Engine/Serialization/Archive/Output/IOutputArchive.h"
#include <string>

namespace Syn
{
    class SYN_API ITomlOutputArchive : public IOutputArchive
    {
    public:
        explicit ITomlOutputArchive(IOutputStream& stream) : IOutputArchive(stream) {}
        virtual ~ITomlOutputArchive() = default;

        virtual std::string ToString() const = 0;
    };
}