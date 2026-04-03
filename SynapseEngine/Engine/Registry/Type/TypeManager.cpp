#include "TypeManager.h"
#include <print>

namespace Syn
{
    TypeManager& TypeManager::Get()
    {
        static TypeManager instance;
        return instance;
    }

    TypeID TypeManager::GetOrRegisterID(const std::string& typeName)
    {
        std::scoped_lock lock(_mtx);

        auto it = _registry.find(typeName);
        if (it != _registry.end())
            return it->second;

        TypeID newID = _counter++;
        _registry[typeName] = newID;

        return newID;
    }
}