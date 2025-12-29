#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/BitFlag.h"
#include <vector>
#include <atomic>

namespace Syn
{
    template<bool Enable>
    struct FlagMixin;

    template<>
    struct FlagMixin<true>
    {
    private:
        struct AtomicByte
        {
            std::atomic<uint8_t> value;

            AtomicByte() : value(0) {}
            AtomicByte(uint8_t v) : value(v) {}
            AtomicByte(const AtomicByte& other) : value(other.value.load(std::memory_order_relaxed)) {}
            AtomicByte& operator=(const AtomicByte& other) {
                value.store(other.value.load(std::memory_order_relaxed), std::memory_order_relaxed);
                return *this;
            }
        };

    public:
        SYN_INLINE void PushFlag();
        SYN_INLINE void PopFlag();
        SYN_INLINE void SwapFlag(DenseIndex a, DenseIndex b);
        SYN_INLINE void FlagIndexChanged(DenseIndex index);
        SYN_INLINE void ClearFlags();

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSet() const;

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBit();

        SYN_INLINE void ResetAllStateBits();

        template<uint32_t... Bits>
        SYN_INLINE bool SetBit(DenseIndex index);

        template<uint32_t... Bits>
        SYN_INLINE void ResetBit(DenseIndex index);

        template<uint32_t... Bits>
        SYN_INLINE bool IsBitSet(DenseIndex index) const;

        SYN_INLINE void ResetAllBits(DenseIndex index);

    protected:
        std::vector<AtomicByte> _flags;
        std::atomic<uint8_t>    _state{ 0 };
    };

    template<>
    struct FlagMixin<false>
    {
        SYN_INLINE void PushFlag() {}
        SYN_INLINE void PopFlag() {}
        SYN_INLINE void SwapFlag(DenseIndex, DenseIndex) {}
        SYN_INLINE void FlagIndexChanged(DenseIndex) {}
        SYN_INLINE void ClearFlags() {}

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSet() const { return false; }

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBit() {}

        SYN_INLINE void ResetAllStateBits() {}

        template<uint32_t... Bits>
        SYN_INLINE bool SetBit(DenseIndex) { return false; }

        template<uint32_t... Bits>
        SYN_INLINE void ResetBit(DenseIndex) {}

        template<uint32_t... Bits>
        SYN_INLINE bool IsBitSet(DenseIndex) const { return false; }

        SYN_INLINE void ResetAllBits(DenseIndex) {}
    };
}

namespace Syn
{
    SYN_INLINE void FlagMixin<true>::PushFlag()
    {
        constexpr uint8_t mask = (1 << REGENERATE_BIT) | (1 << UPDATE_BIT) | (1 << INDEX_CHANGED_BIT);

        _flags.emplace_back(mask);
        _state.fetch_or(mask, std::memory_order_relaxed);
    }

    SYN_INLINE void FlagMixin<true>::PopFlag()
    {
        SYN_ASSERT(!_flags.empty(), "Attempting to pop from empty flag vector");
        _flags.pop_back();
    }

    SYN_INLINE void FlagMixin<true>::SwapFlag(DenseIndex a, DenseIndex b)
    {
        SYN_ASSERT(a < _flags.size() && b < _flags.size(), "Flag index out of bounds");

        uint8_t valA = _flags[a].value.load(std::memory_order_relaxed);
        uint8_t valB = _flags[b].value.load(std::memory_order_relaxed);

        _flags[a].value.store(valB, std::memory_order_relaxed);
        _flags[b].value.store(valA, std::memory_order_relaxed);
    }

    SYN_INLINE void FlagMixin<true>::FlagIndexChanged(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        constexpr uint8_t mask = (1 << INDEX_CHANGED_BIT);

        _flags[index].value.fetch_or(mask, std::memory_order_relaxed);
        _state.fetch_or(mask, std::memory_order_relaxed);
    }

    SYN_INLINE void FlagMixin<true>::ClearFlags()
    {
        _flags.clear();
        _state.store(0, std::memory_order_relaxed);
    }

    template<uint32_t... Bits>
    SYN_INLINE bool FlagMixin<true>::IsStateBitSet() const
    {
        constexpr uint8_t mask = ((1 << Bits) | ...);
        return (_state.load(std::memory_order_relaxed) & mask) == mask;
    }

    template<uint32_t... Bits>
    SYN_INLINE void FlagMixin<true>::ResetStateBit()
    {
        constexpr uint8_t mask = ((1 << Bits) | ...);
        _state.fetch_and(~mask, std::memory_order_relaxed);
    }

    SYN_INLINE void FlagMixin<true>::ResetAllStateBits()
    {
        _state.store(0, std::memory_order_relaxed);
    }

    template<uint32_t... Bits>
    SYN_INLINE bool FlagMixin<true>::SetBit(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        constexpr uint8_t mask = ((1 << Bits) | ...);

        uint8_t prev = _flags[index].value.fetch_or(mask, std::memory_order_relaxed);
        _state.fetch_or(mask, std::memory_order_relaxed);

        return (prev & mask) != 0;
    }

    template<uint32_t... Bits>
    SYN_INLINE void FlagMixin<true>::ResetBit(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        constexpr uint8_t mask = ((1 << Bits) | ...);

        _flags[index].value.fetch_and(~mask, std::memory_order_relaxed);
    }

    template<uint32_t... Bits>
    SYN_INLINE bool FlagMixin<true>::IsBitSet(DenseIndex index) const
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        constexpr uint8_t mask = ((1 << Bits) | ...);

        return (_flags[index].value.load(std::memory_order_relaxed) & mask) == mask;
    }

    SYN_INLINE void FlagMixin<true>::ResetAllBits(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        _flags[index].value.store(0, std::memory_order_relaxed);
    }
}