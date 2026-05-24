#pragma once
#include "IOutputArchive.h"
#include <string>

namespace Syn
{
    class SYN_API IJsonOutputArchive : public IOutputArchive {
    public:
        explicit IJsonOutputArchive(IOutputStream& stream) : IOutputArchive(stream) {}
        virtual ~IJsonOutputArchive() = default;
        virtual std::string ToString() const = 0;
    };
}