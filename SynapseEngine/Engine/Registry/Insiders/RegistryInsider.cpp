#include "RegistryInsider.h"
#include "Engine/Registry/Registry.h"

namespace Syn
{
    EntityID& RegistryInsider::GetEntityCounter(Registry& reg, Passkey<RegistryInsider>) {
        return reg._entityCounter;
    }

    std::vector<EntityID>& RegistryInsider::GetFreeEntities(Registry& reg, Passkey<RegistryInsider>) {
        return reg._freeEntities;
    }

    SparseSet& RegistryInsider::GetActiveEntities(Registry& reg, Passkey<RegistryInsider>) {
        return reg._activeEntities;
    }
}