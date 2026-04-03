#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "MappingExtension.h"
#include "../../../Entity.h"
#include "../Core/SparseVectorMapping.h"
#include <span>

namespace Syn
{
    template<typename DerivedPool>
    struct SparseVectorMappingExtension
    {
    private:
        SYN_INLINE DerivedPool& AsDerived() { return static_cast<DerivedPool&>(*this); }
        SYN_INLINE const DerivedPool& AsDerived() const { return static_cast<const DerivedPool&>(*this); }
    public:
        SYN_INLINE std::span<const DenseIndex> GetSparseIndices() const
        {
            return AsDerived().GetMapping().GetSparseIndices();
        }
    };

    template<>
    struct MappingTraits<SparseVectorMapping>
    {
        template<typename PoolType>
        using Extension = SparseVectorMappingExtension<PoolType>;
    };
}
