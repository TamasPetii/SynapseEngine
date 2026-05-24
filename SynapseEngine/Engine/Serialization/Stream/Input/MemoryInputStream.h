#pragma once
#include "Engine/SynApi.h"
#include "IInputStream.h"
#include <span>
#include <cstdint>

namespace Syn
{
    class SYN_API MemoryInputStream : public IInputStream
    {
    public:
        explicit MemoryInputStream(std::span<const uint8_t> data);
        ~MemoryInputStream() override = default;

        void ReadRaw(void* data, size_t size) override;
        bool IsValid() const override;
    private:
        std::span<const uint8_t> _data;
        size_t _offset;
        bool _isValid;
    };
}