#include "PipelineOverrideSystem.h"

namespace Syn
{
    std::vector<TypeID> PipelineOverrideSystem::GetWriteDependencies() const {
        return { TypeInfo<PipelineOverrideSystem>::ID };
    }
}