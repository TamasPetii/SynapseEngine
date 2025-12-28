#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "MappingCRTP.h"
#include <vector>

namespace Syn
{
    class SYN_API SparseVectorMapping : public MappingCRTP<SparseVectorMapping>
    {
    public:
        // Common mapping interface is not used to avoid virtual calls in performance-critical code
        void Set(EntityID entity, DenseIndex index);
        DenseIndex Get(EntityID entity) const;
        void Remove(EntityID entity);
        bool Contains(EntityID entity) const;
        void Clear();
    private:
        std::vector<DenseIndex> _indices;
    };
}

namespace Syn
{
    SYN_INLINE void SparseVectorMapping::Set(EntityID entity, DenseIndex index)
    {
        if (entity >= _indices.size())
        {
            _indices.resize(entity + 1, NULL_INDEX);
        }

        //SYN_ASSERT(_indices[entity] == NULL_INDEX, "Entity already mapped in VectorMapping");

        _indices[entity] = index;
    }

    SYN_INLINE DenseIndex SparseVectorMapping::Get(EntityID entity) const
    {
        SYN_ASSERT(entity < _indices.size(), "Entity ID out of bounds");
        SYN_ASSERT(_indices[entity] != NULL_INDEX, "Entity ID not mapped");
        return _indices[entity];
    }

    SYN_INLINE void SparseVectorMapping::Remove(EntityID entity)
    {
        SYN_ASSERT(entity < _indices.size(), "Entity ID out of bounds during remove");
        _indices[entity] = NULL_INDEX;
    }

    SYN_INLINE bool SparseVectorMapping::Contains(EntityID entity) const
    {
        return entity < _indices.size() && _indices[entity] != NULL_INDEX;
    }

    SYN_INLINE void SparseVectorMapping::Clear()
    {
        _indices.clear();
    }
}