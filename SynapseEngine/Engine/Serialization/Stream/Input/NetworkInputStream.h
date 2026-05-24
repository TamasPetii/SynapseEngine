#pragma once
#include "Engine/SynApi.h"
#include "IInputStream.h"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace Syn
{
    class SYN_API NetworkInputStream : public IInputStream
    {
    public:
        NetworkInputStream();
        ~NetworkInputStream() override = default;

        void ReadRaw(void* data, size_t size) override;
        bool IsValid() const override;
        void AppendPacket(const void* data, size_t size);
        void Reset();
    private:
        std::vector<uint8_t> _buffer;
        size_t _readOffset;
        bool _isValid;
    };
}