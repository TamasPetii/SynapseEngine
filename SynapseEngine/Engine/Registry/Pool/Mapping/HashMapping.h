#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "MappingCRTP.h"
#include <unordered_map>

namespace Syn
{
    class SYN_API HashMapping : public MappingCRTP<HashMapping>
    {
    public:
        // Common mapping interface is not used to avoid virtual calls in performance-critical code
        void Set(EntityID entity, DenseIndex index);
        DenseIndex Get(EntityID entity) const;
        void Remove(EntityID entity);
        bool Contains(EntityID entity) const;
        void Clear();
    private:
        std::unordered_map<EntityID, DenseIndex> _map;
    };
}

namespace Syn
{
    SYN_INLINE void HashMapping::Set(EntityID entity, DenseIndex index)
    {
        //SYN_ASSERT(!_map.contains(entity), "Entity already present in HashMapping");
        _map[entity] = index;
    }

    SYN_INLINE DenseIndex HashMapping::Get(EntityID entity) const
    {
        SYN_ASSERT(_map.contains(entity), "Entity not found in HashMapping");
        return _map.at(entity);
    }

    SYN_INLINE void HashMapping::Remove(EntityID entity)
    {
        SYN_ASSERT(_map.contains(entity), "Attempting to remove non-existent entity from HashMapping");
        _map.erase(entity);
    }

    SYN_INLINE bool HashMapping::Contains(EntityID entity) const
    {
        return _map.find(entity) != _map.end();
    }

    SYN_INLINE void HashMapping::Clear()
    {
        _map.clear();
    }
}