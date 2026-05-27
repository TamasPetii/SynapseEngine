#pragma once
#include "Engine/SynApi.h"
#include "IOutputStream.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    class SYN_API MemoryOutputStream : public IOutputStream
    {
    public:
        explicit MemoryOutputStream(size_t reserveSize = 1024);
        ~MemoryOutputStream() override = default;

        void WriteRaw(const void* data, size_t size) override;
        bool IsValid() const override;

        const std::vector<uint8_t>& GetBuffer() const;
        std::vector<uint8_t> ConsumeBuffer();
    private:
        std::vector<uint8_t> _buffer;
    };
}