#pragma once
#include "Engine/SynApi.h"
#include "IInputStream.h"
#include <filesystem>
#include <fstream>

namespace Syn
{
    class SYN_API FileInputStream : public IInputStream
    {
    public:
        explicit FileInputStream(const std::filesystem::path& path);
        ~FileInputStream() override;

        void ReadRaw(void* data, size_t size) override;
        bool IsValid() const override;
        size_t GetSize() override;
    private:
        std::ifstream _stream;
    };
}