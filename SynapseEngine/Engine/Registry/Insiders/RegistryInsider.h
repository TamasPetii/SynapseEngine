#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/Pool/PoolTypes.h"
#include <vector>

namespace Syn
{
    class Registry;

    class SYN_API RegistryInsider
    {
    public:
        static Passkey<RegistryInsider> GetKey() { return {}; }

        static EntityID& GetEntityCounter(Registry& reg, Passkey<RegistryInsider>);
        static std::vector<EntityID>& GetFreeEntities(Registry& reg, Passkey<RegistryInsider>);
        static SparseSet& GetActiveEntities(Registry& reg, Passkey<RegistryInsider>);
    };
}