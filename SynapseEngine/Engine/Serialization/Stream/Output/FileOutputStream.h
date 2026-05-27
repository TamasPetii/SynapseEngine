#pragma once
#include "Engine/SynApi.h"
#include "IOutputStream.h"
#include <filesystem>
#include <fstream>

namespace Syn
{
    class SYN_API FileOutputStream : public IOutputStream
    {
    public:
        explicit FileOutputStream(const std::filesystem::path& path);
        ~FileOutputStream() override;

        void WriteRaw(const void* data, size_t size) override;
        bool IsValid() const override;
    private:
        std::ofstream _stream;
    };
}