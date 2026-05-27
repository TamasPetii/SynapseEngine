#pragma once
#include "Engine/Serialization/Archive/Input/IInputArchive.h"

namespace Syn
{
    class SYN_API ITomlInputArchive : public IInputArchive {
    public:
        explicit ITomlInputArchive(IInputStream& stream) : IInputArchive(stream) {}
        virtual ~ITomlInputArchive() = default;
    };
}