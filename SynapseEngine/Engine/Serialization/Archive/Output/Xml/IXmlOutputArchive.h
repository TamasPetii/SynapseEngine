#pragma once
#include "Engine/Serialization/Archive/Output/IOutputArchive.h"
#include <string>

namespace Syn
{
    class SYN_API IXmlOutputArchive : public IOutputArchive
    {
    public:
        explicit IXmlOutputArchive(IOutputStream& stream) : IOutputArchive(stream) {}
        virtual ~IXmlOutputArchive() = default;

        virtual std::string ToString() const = 0;
    };
}