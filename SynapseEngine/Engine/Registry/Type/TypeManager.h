#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <unordered_map>
#include <mutex>

namespace Syn
{
    using TypeID = uint32_t;

    class SYN_API TypeManager {
    public:
        TypeManager(const TypeManager&) = delete;
        TypeManager& operator=(const TypeManager&) = delete;

        static TypeManager& Get();
        TypeID GetOrRegisterID(const std::string& typeName);
    private:
        TypeManager() = default;
        TypeID _counter = 0;
        std::mutex _mtx;
        std::unordered_map<std::string, TypeID> _registry;
    };
}