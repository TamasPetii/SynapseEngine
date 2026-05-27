#include "SparseMappingInsider.h"
#include "Engine/Registry/Pool/Mapping/Core/SparseVectorMapping.h"

namespace Syn
{
    std::vector<DenseIndex>& SparseMappingInsider::GetIndices(SparseVectorMapping& svm, Passkey<SparseMappingInsider>) {
        return svm._indices;
    }
}