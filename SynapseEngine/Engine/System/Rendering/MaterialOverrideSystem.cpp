#include "MaterialOverrideSystem.h"

namespace Syn
{
    std::vector<TypeID> MaterialOverrideSystem::GetWriteDependencies() const {
        return { TypeInfo<MaterialOverrideSystem>::ID };
    }
}