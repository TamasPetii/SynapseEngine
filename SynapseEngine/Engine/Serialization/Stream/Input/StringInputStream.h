#pragma once
#include "Engine/SynApi.h"
#include "IInputStream.h"
#include <string>

namespace Syn
{
    class SYN_API StringInputStream : public IInputStream
    {
    public:
        explicit StringInputStream(std::string data);
        ~StringInputStream() override = default;

        void ReadRaw(void* data, size_t size) override;
        bool IsValid() const override;
    private:
        std::string _data;
        size_t _position = 0;
    };
}