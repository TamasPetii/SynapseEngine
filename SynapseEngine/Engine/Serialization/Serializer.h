#pragma once
#include "Engine/SynApi.h"
#include "Engine/Serialization/Archive/IArchiveRegistry.h"
#include "Engine/Serialization/Stream/Output/StringOutputStream.h"
#include "Engine/Serialization/Stream/Output/FileOutputStream.h"
#include "Engine/Serialization/Stream/Input/FileInputStream.h"
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/DefaultSerializationService.h"
#include "Engine/ServiceLocator.h"
#include <filesystem>
#include <memory>
#include <string>

namespace Syn
{
    class SYN_API Serializer {
    public:
        explicit Serializer(std::unique_ptr<DefaultSerializationService> service)
            : _service(std::move(service)) {}

        template<typename T>
        std::future<bool> SaveToFileAsync(std::filesystem::path path, const T& data) {
            auto executor = ServiceLocator::Get<tf::Executor>();

            return executor->async([this, p = path, d = data]() -> bool {
                return this->SaveToFile(p, d);
                });
        }

        template<typename T>
        bool SaveToFile(const std::filesystem::path& path, const T& data) {
            FileOutputStream stream(path);
            if (!stream.IsValid()) return false;

            _service->Save(stream, path.extension().string(), data);
            return true;
        }

        template<typename T>
        std::string SaveToString(const std::string& extension, const T& data) {
            StringOutputStream stream;

            _service->Save(stream, extension, data);
            return stream.ConsumeString();
        }

        template<typename T>
        bool LoadFromFile(const std::filesystem::path& path, T& outData) {
            FileInputStream stream(path);
            if (!stream.IsValid()) return false;

            _service->Load(stream, path.extension().string(), outData);
            return true;
        }

        template<typename T>
        std::future<std::optional<T>> LoadFromFileAsync(std::filesystem::path path) {
            auto executor = ServiceLocator::Get<tf::Executor>();

            return executor->async([this, p = path]() -> std::optional<T> {
                T loadedData{};
                if (this->LoadFromFile(p, loadedData)) {
                    return loadedData;
                }
                return std::nullopt;
                });
        }
    private:
        std::unique_ptr<DefaultSerializationService> _service;
    };
}