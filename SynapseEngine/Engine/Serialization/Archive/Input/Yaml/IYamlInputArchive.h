#pragma once
#include "Engine/Serialization/Archive/Input/IInputArchive.h"

namespace Syn
{
    class SYN_API IYamlInputArchive : public IInputArchive {
    public:
        explicit IYamlInputArchive(IInputStream& stream) : IInputArchive(stream) {}
        virtual ~IYamlInputArchive() = default;
    };
}