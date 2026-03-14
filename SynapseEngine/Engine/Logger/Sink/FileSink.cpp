#include "FileSink.h"
#include "Engine/Logger/LogUtils.h"
#include <filesystem>
#include <format>

namespace Syn
{
    FileSink::FileSink() {
        const char* appDataPath = std::getenv("APPDATA");
        std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
        std::filesystem::path logDir = baseDir / "Synapse" / "Logs";

        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directories(logDir);
        }

        std::string filename = std::format("{}/SynapseEngine_{}.log",
            logDir.string(),
            Syn::LogUtils::GetCurrentTimeForFileName());

        _file.open(filename, std::ios::out);
    }

    void FileSink::Log(const LogMessage& msg) {
        std::scoped_lock lock(_mtx);

        if (_file.is_open()) {
            _file << msg.ToString();
            _file.flush();
        }
    }
}