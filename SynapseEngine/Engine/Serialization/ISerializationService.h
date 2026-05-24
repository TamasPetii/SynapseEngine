#pragma once
#include "Engine/SynApi.h"
#include "Engine/Serialization/Archive/IArchiveRegistry.h"

namespace Syn
{
    template <typename Derived>
    class SYN_API ISerializationService {
    public:
        IArchiveRegistry& GetRegistry() {
            return static_cast<Derived*>(this)->GetRegistryImpl();
        }

        template<typename T>
        void Load(IInputStream& stream, const std::string& extension, T& outData) {
            static_cast<Derived*>(this)->template LoadImpl<T>(stream, extension, outData);
        }

        template<typename T>
        void Save(IOutputStream& stream, const std::string& extension, const T& data) {
            static_cast<Derived*>(this)->template SaveImpl<T>(stream, extension, data);
        }
    };
}