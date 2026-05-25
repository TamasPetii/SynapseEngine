#pragma once
#include "Engine/Serialization/Archive/Input/IInputArchive.h"
#include <string>

namespace Syn
{
    class SYN_API IJsonInputArchive : public IInputArchive {
    public:
        explicit IJsonInputArchive(IInputStream& stream) : IInputArchive(stream) {}
        virtual ~IJsonInputArchive() = default;
    };
}