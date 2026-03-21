#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

#include "../../../Entity.h"
#include "../Utils/MappingUtils.h"
#include "../Utils/MappingCRTP.h"

#include <vector>
#include <memory>
#include <algorithm> 

namespace Syn
{
	class SYN_API SparsePagedMapping : public MappingCRTP<SparsePagedMapping>
    {
    public:
        static constexpr uint32_t PAGE_SIZE = 1024;
        static constexpr uint32_t PAGE_SHIFT = ConstLog2(PAGE_SIZE);
        static constexpr uint32_t PAGE_MASK = PAGE_SIZE - 1;
    public:
        SparsePagedMapping() = default;
        SparsePagedMapping(SparsePagedMapping&&) noexcept = default;
        SparsePagedMapping& operator=(SparsePagedMapping&&) noexcept = default;
        SparsePagedMapping(const SparsePagedMapping&) = delete;
        SparsePagedMapping& operator=(const SparsePagedMapping&) = delete;
    public:
        SYN_INLINE void Set(EntityID entity, DenseIndex index);
        SYN_INLINE DenseIndex Get(EntityID entity) const;
        SYN_INLINE void Remove(EntityID entity);
        SYN_INLINE bool Contains(EntityID entity) const;
        SYN_INLINE void Clear();
        SYN_INLINE void EnsureEntityMapping(EntityID entity);
    private:
        SYN_INLINE uint32_t CalculatePageIndex(EntityID entity) const;
        SYN_INLINE uint32_t CalculatePageOffset(EntityID entity) const;
        SYN_INLINE void EnsurePageExists(uint32_t pageIndex);
    private:
        std::vector<std::unique_ptr<DenseIndex[]>> _pages;
    };
}

namespace Syn
{
    SYN_INLINE uint32_t SparsePagedMapping::CalculatePageIndex(EntityID entity) const
    {
        return entity >> PAGE_SHIFT;
    }

    SYN_INLINE uint32_t SparsePagedMapping::CalculatePageOffset(EntityID entity) const
    {
        return entity & PAGE_MASK;
    }

    SYN_INLINE void SparsePagedMapping::EnsurePageExists(uint32_t pageIndex)
    {
        if (pageIndex >= _pages.size())
        {
            _pages.resize(pageIndex + 1);
        }

        if (!_pages[pageIndex])
        {
            _pages[pageIndex] = std::make_unique<DenseIndex[]>(PAGE_SIZE);
            std::fill_n(_pages[pageIndex].get(), PAGE_SIZE, NULL_INDEX);
        }
    }

    SYN_INLINE void SparsePagedMapping::Set(EntityID entity, DenseIndex index)
    {
        const uint32_t pageIndex = CalculatePageIndex(entity);
        const uint32_t offset = CalculatePageOffset(entity);

        EnsurePageExists(pageIndex);

        //SYN_ASSERT(_pages[pageIndex][offset] == NULL_INDEX || _pages[pageIndex][offset] == index, "Entity already mapped to a different index!");
           
        _pages[pageIndex][offset] = index;
    }

    SYN_INLINE DenseIndex SparsePagedMapping::Get(EntityID entity) const
    {
        const uint32_t pageIndex = CalculatePageIndex(entity);
        const uint32_t offset = CalculatePageOffset(entity);

        SYN_ASSERT(pageIndex < _pages.size() && _pages[pageIndex], "Entity ID not mapped (Page missing)");

        const DenseIndex index = _pages[pageIndex][offset];

        SYN_ASSERT(index != NULL_INDEX, "Entity ID mapped to NULL_INDEX");

        return index;
    }

    SYN_INLINE void SparsePagedMapping::Remove(EntityID entity)
    {
        const uint32_t pageIndex = CalculatePageIndex(entity);

        if (pageIndex >= _pages.size() || !_pages[pageIndex])
        {
            SYN_ASSERT(false, "Attempting to remove unmapped entity");
            return;
        }

        const uint32_t offset = CalculatePageOffset(entity);
        _pages[pageIndex][offset] = NULL_INDEX;
    }

    SYN_INLINE bool SparsePagedMapping::Contains(EntityID entity) const
    {
        const uint32_t pageIndex = CalculatePageIndex(entity);

        if (pageIndex >= _pages.size() || !_pages[pageIndex])
            return false;

        return _pages[pageIndex][CalculatePageOffset(entity)] != NULL_INDEX;
    }

    SYN_INLINE void SparsePagedMapping::Clear()
    {
        _pages.clear();
    }

    SYN_INLINE void SparsePagedMapping::EnsureEntityMapping(EntityID entity)
    {
        const uint32_t pageIndex = CalculatePageIndex(entity);

        if (pageIndex >= _pages.size())
        {
            _pages.resize(pageIndex + 1);
        }

        if (!_pages[pageIndex])
        {
            _pages[pageIndex] = std::make_unique<DenseIndex[]>(PAGE_SIZE);
            std::fill_n(_pages[pageIndex].get(), PAGE_SIZE, NULL_INDEX);
        }
    }
}