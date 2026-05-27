#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/Pool/Storage/Core/StorageBackend.h"
#include <vector>

namespace Syn
{
    class SYN_API StorageBackendInsider
    {
    public:
        static Passkey<StorageBackendInsider> GetKey() { return {}; }

        template<typename T, typename F>
        static std::vector<EntityID>& GetEntities(StorageBackend<T, F>& sb, Passkey<StorageBackendInsider>) {
            return sb._entities;
        }
    };
}