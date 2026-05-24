#pragma once
#include "Engine/SynApi.h"
#include "IOutputStream.h"
#include <vector>
#include <cstdint>
#include <functional>

namespace Syn
{
    using SendPacketCallback = std::function<void(const void* data, size_t size)>;

    class SYN_API NetworkOutputStream : public IOutputStream
    {
    public:
        explicit NetworkOutputStream(SendPacketCallback sendCallback, size_t maxPacketSize = 1400);
        ~NetworkOutputStream() override;

        void WriteRaw(const void* data, size_t size) override;
        bool IsValid() const override;
        void Flush();
    private:
        SendPacketCallback _sendCallback;
        size_t _maxPacketSize;
        std::vector<uint8_t> _buffer;
    };
}