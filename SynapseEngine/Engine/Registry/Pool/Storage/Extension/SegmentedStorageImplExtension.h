// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "StorageExtension.h"
#include "../../../Entity.h"
#include "../Core/SegmentedStorageImpl.h"

namespace Syn
{
    template<typename DerivedPool>
    struct SegmentedStorageImplExtension
    {
    private:
        SYN_INLINE DerivedPool& AsDerived() { return static_cast<DerivedPool&>(*this); }
        SYN_INLINE const DerivedPool& AsDerived() const { return static_cast<const DerivedPool&>(*this); }
    public:

        SYN_INLINE void SetCategory(EntityID entity, StorageCategory newCat)
        {
            auto& pool = AsDerived();
            SYN_ASSERT(pool.GetMapping().Contains(entity), "Entity not in pool");

            const DenseIndex index = pool.GetMapping().Get(entity);

            pool.GetStorage().SetCategory(index, newCat, [&pool](EntityID movedEntity, DenseIndex newIndex) {
                pool.GetMapping().Set(movedEntity, newIndex);
                });
        }

        SYN_INLINE StorageCategory GetCategory(EntityID entity) const
        {
            const auto& pool = AsDerived();
            const DenseIndex index = pool.GetMapping().Get(entity);
            return pool.GetStorage().GetCategory(index);
        }

        SYN_INLINE void MarkStaticDirty(EntityID entity)
        {
            const DenseIndex index = AsDerived().GetMapping().Get(entity);
            return AsDerived().GetStorage().MarkStaticDirty(index);
        }

        SYN_INLINE std::span<const EntityID> GetDirtyStatics() const
        {
            return AsDerived().GetStorage().GetDirtyStatics();
        }

        SYN_INLINE std::span<const EntityID> GetStaticEntities() const
        {
            return AsDerived().GetStorage().GetStaticEntities();
        }

        SYN_INLINE std::span<const EntityID> GetDynamicEntities() const
        {
            return AsDerived().GetStorage().GetDynamicEntities();
        }

        SYN_INLINE std::span<const EntityID> GetStreamEntities() const
        {
            return AsDerived().GetStorage().GetStreamEntities();
        }

        SYN_INLINE void ResetStaticDirtyCounter()
        {
            AsDerived().GetStorage().ResetStaticDirtyCounter();
        }

        SYN_INLINE bool IsStatic(EntityID entity) const {
            DenseIndex index = AsDerived().GetMapping().Get(entity);
            return AsDerived().GetStorage().IsStatic(index);
        }

        SYN_INLINE bool IsDynamic(EntityID entity) const {
            DenseIndex index = AsDerived().GetMapping().Get(entity);
            return AsDerived().GetStorage().IsDynamic(index);
        }

        SYN_INLINE bool IsStream(EntityID entity) const {
            DenseIndex index = AsDerived().GetMapping().Get(entity);
            return AsDerived().GetStorage().IsStream(index);
        }

        template<typename U> 
            requires (!std::is_void_v<U>)
        SYN_INLINE void UpdateStaticData(std::span<const U> sortedData)
        {
            AsDerived().GetStorage().UpdateStaticData(sortedData);
        }

        SYN_INLINE void UpdateStaticEntities(std::span<const EntityID> sortedEntities)
        {
            AsDerived().GetStorage().UpdateStaticEntities(sortedEntities);
        }

        SYN_INLINE void RebuildStaticIndices(std::span<const EntityID> sortedEntities)
        {
            auto& pool = AsDerived();
            SYN_ASSERT(sortedEntities.size() == pool.GetStorage().GetStaticEntities().size(), "Error: Sorted entity array size differs from the static region size!");

            auto& mapping = pool.GetMapping();

            for (size_t i = 0; i < sortedEntities.size(); ++i)
            {
                mapping.Set(sortedEntities[i], static_cast<DenseIndex>(i));
            }
        }
    };

    template<typename T>
    struct StorageTraits<SegmentedStorageImpl<T>>
    {
        template<typename PoolType>
        using Extension = SegmentedStorageImplExtension<PoolType>;
    };
}