#pragma once
#include "Engine/SynApi.h"
#include <cstddef>

namespace Syn
{
    class SYN_API IOutputStream
    {
    public:
        virtual ~IOutputStream() = default;

        virtual void WriteRaw(const void* data, size_t size) = 0;
        virtual bool IsValid() const = 0;
    };
}