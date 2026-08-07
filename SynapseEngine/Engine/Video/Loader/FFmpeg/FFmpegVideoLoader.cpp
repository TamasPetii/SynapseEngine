#include "FFmpegVideoLoader.h"

namespace Syn
{
    std::unique_ptr<IVideoState> FFmpegVideoLoader::OpenFile(const std::filesystem::path& path)
    {
        auto state = std::make_unique<FFmpegVideoState>(path.string());
        if (state->GetInfo().width == 0) {
            return nullptr;
        }
        return state;
    }

    std::unique_ptr<IVideoState> FFmpegVideoLoader::OpenMemory(const std::vector<uint8_t>& data)
    {
        return nullptr;
    }

    std::vector<std::string> FFmpegVideoLoader::GetSupportedExtensions() const
    {
        return { ".mp4", ".mkv", ".avi", ".mov", ".webm" };
    }
}