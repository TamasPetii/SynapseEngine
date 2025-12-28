#include "FileSink.h"
#include "Engine/Logger/LogUtils.h"
#include <filesystem>
#include <format>

namespace Syn
{
    FileSink::FileSink() {
        std::filesystem::path logDir = "Logs";
        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        std::string filename = std::format("Logs/Engine_{}.log", Syn::LogUtils::GetCurrentTimeForFileName());

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