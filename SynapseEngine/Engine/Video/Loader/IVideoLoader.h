#pragma once
#include "Engine/SynApi.h"
#include "IVideoState.h"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

namespace Syn
{
    class SYN_API IVideoLoader
    {
    public:
        virtual ~IVideoLoader() = default;
        virtual std::unique_ptr<IVideoState> OpenFile(const std::filesystem::path& path) = 0;
        virtual std::unique_ptr<IVideoState> OpenMemory(const std::vector<uint8_t>& data) = 0;
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
    };
}