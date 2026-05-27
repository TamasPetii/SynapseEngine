#pragma once
#include "Engine/Serialization/Archive/Output/IOutputArchive.h"
#include <string>

namespace Syn
{
    class SYN_API IYamlOutputArchive : public IOutputArchive
    {
    public:
        explicit IYamlOutputArchive(IOutputStream& stream) : IOutputArchive(stream) {}
        virtual ~IYamlOutputArchive() = default;

        virtual std::string ToString() const = 0;
    };
}