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
            auto& pool = AsDerived();
            const DenseIndex index = pool.GetMapping().Get(entity);
            pool.GetStorage().MarkStaticDirty(index);
        }

        SYN_INLINE std::span<EntityID> GetDirtyStatics()
        {
            return AsDerived().GetStorage().GetDirtyStatics();
        }

        SYN_INLINE void ResetStaticDirtyCounter()
        {
            AsDerived().GetStorage().ResetStaticDirtyCounter();
        }
    };

    template<typename T>
    struct StorageTraits<SegmentedStorageImpl<T>>
    {
        template<typename PoolType>
        using Extension = SegmentedStorageImplExtension<PoolType>;
    };
}