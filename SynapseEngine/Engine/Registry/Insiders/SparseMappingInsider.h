#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include "Engine/Registry/Entity.h"
#include <vector>

#include "Engine/Registry/Pool/Mapping/Core/SparseVectorMapping.h"

namespace Syn
{
    class SparseVectorMapping;

    class SYN_API SparseMappingInsider
    {
    public:
        static Passkey<SparseMappingInsider> GetKey() { return {}; }

        static std::vector<DenseIndex>& GetIndices(SparseVectorMapping& svm, Passkey<SparseMappingInsider>);
    };
}