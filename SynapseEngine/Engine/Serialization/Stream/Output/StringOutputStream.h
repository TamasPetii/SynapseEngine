#pragma once
#include "Engine/SynApi.h"
#include "IOutputStream.h"
#include <string>

namespace Syn
{
    class SYN_API StringOutputStream : public IOutputStream
    {
    public:
        StringOutputStream() = default;
        ~StringOutputStream() override = default;

        void WriteRaw(const void* data, size_t size) override;
        bool IsValid() const override;

        const std::string& GetString() const { return _buffer; }
        std::string ConsumeString();
    private:
        std::string _buffer;
    };
}