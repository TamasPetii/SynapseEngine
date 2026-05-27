#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    template <typename T>
    class Passkey
    {
    public:
        Passkey(const Passkey&) = default;
        Passkey& operator=(const Passkey&) = default;
        Passkey(Passkey&&) = default;
        Passkey& operator=(Passkey&&) = default;
    private:
        friend T;
        Passkey() = default;
    };
}