#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h" 

#include "Engine/Registry/Pool/Mapping/Core/SparseVectorMapping.h"
#include "Engine/Registry/Insiders/SparseMappingInsider.h"

namespace Syn
{
    template<>
    struct Schema<SparseVectorMapping> {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);
            auto& indices = SparseMappingInsider::GetIndices(v, SparseMappingInsider::GetKey());

            if (ar.IsBinary())
            {
                BlitVector<DenseIndex> blitIdx{ indices };
                ar.Property("indices", blitIdx);
            }
            else {
                ar.Property("indices", indices);
            }
        }
    };
}