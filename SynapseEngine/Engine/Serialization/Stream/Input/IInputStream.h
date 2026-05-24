#pragma once
#include "Engine/SynApi.h"
#include <cstddef>

namespace Syn
{
    class SYN_API IInputStream
    {
    public:
        virtual ~IInputStream() = default;

        virtual void ReadRaw(void* data, size_t size) = 0;
        virtual bool IsValid() const = 0;
        virtual size_t GetSize() = 0;
    };
}