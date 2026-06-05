#pragma once
#include "ISerializationService.h"
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Stream/Input/IInputStream.h"
#include "Engine/Serialization/Stream/Output/IOutputStream.h"
#include <print>

namespace Syn {
    class SYN_API DefaultSerializationService : public ISerializationService<DefaultSerializationService> {
    public:
        DefaultSerializationService(std::unique_ptr<IArchiveRegistry> registry)
            : _registry(std::move(registry)) {}

        IArchiveRegistry& GetRegistryImpl() { return *_registry; }

        template<typename T>
        void LoadImpl(IInputStream& stream, const std::string& extension, T& outData) {
            auto archive = _registry->CreateInput(extension, stream);
            if (archive) {
                archive->Deserialize();
                Schema<T>::Invoke(*archive, "Root", outData);
            }
        }

        template<typename T>
        void SaveImpl(IOutputStream& stream, const std::string& extension, const T& data) {
            auto archive = _registry->CreateOutput(extension, stream);
            if (archive) {
                Schema<std::remove_cvref_t<T>>::Invoke(*archive, "Root", data);
                archive->Serialize();
            }
        }
    private:
        std::unique_ptr<IArchiveRegistry> _registry;
    };
}