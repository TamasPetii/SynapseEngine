#pragma once
#include "Engine/Serialization/Archive/Input/IInputArchive.h"

namespace Syn
{
    class SYN_API IXmlInputArchive : public IInputArchive {
    public:
        explicit IXmlInputArchive(IInputStream& stream) : IInputArchive(stream) {}
        virtual ~IXmlInputArchive() = default;
    };
}