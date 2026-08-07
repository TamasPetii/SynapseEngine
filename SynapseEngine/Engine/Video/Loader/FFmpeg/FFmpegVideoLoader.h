#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Loader/IVideoLoader.h"
#include "FFmpegVideoState.h"

namespace Syn
{
    class SYN_API FFmpegVideoLoader : public IVideoLoader
    {
    public:
        FFmpegVideoLoader() = default;
        ~FFmpegVideoLoader() override = default;

        std::unique_ptr<IVideoState> OpenFile(const std::filesystem::path& path) override;
        std::unique_ptr<IVideoState> OpenMemory(const std::vector<uint8_t>& data) override;
        std::vector<std::string> GetSupportedExtensions() const override;
    };
}