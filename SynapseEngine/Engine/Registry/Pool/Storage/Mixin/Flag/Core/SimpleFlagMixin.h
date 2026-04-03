#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/BitFlag.h"
#include "../Utils/FlagMixinCRTP.h"
#include <vector>
#include <cstdint>
#include <atomic>

namespace Syn
{
    struct SimpleFlagMixin : public FlagMixinCRTP<SimpleFlagMixin>
    {
        SYN_INLINE void PushFlagImpl() {
            constexpr uint32_t mask = (1 << REGENERATE_BIT) | (1 << UPDATE_BIT) | (1 << INDEX_CHANGED_BIT);
            _flags.push_back(mask);
            _state.fetch_or(mask, std::memory_order_relaxed);
        }

        SYN_INLINE void PopFlagImpl()
        {
            _flags.pop_back();
        }

        SYN_INLINE void SwapFlagImpl(DenseIndex a, DenseIndex b)
        {
            std::swap(_flags[a], _flags[b]);
        }

        SYN_INLINE void FlagIndexChangedImpl(DenseIndex index) {
            constexpr uint32_t mask = (1 << INDEX_CHANGED_BIT);
            _flags[index] |= mask;
            _state.fetch_or(mask, std::memory_order_relaxed);
        }

        SYN_INLINE void ClearFlagsImpl()
        {
            _flags.clear();
            _state.store(0, std::memory_order_relaxed);
        }

        template<uint32_t... Bits>
        SYN_INLINE bool SetBitImpl(DenseIndex index) {
            constexpr uint32_t mask = ((1 << Bits) | ...);
            uint32_t prev = _flags[index];
            _flags[index] |= mask;

            _state.fetch_or(mask, std::memory_order_relaxed);

            return (prev & mask) != 0;
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetBitImpl(DenseIndex index) {
            constexpr uint32_t mask = ((1 << Bits) | ...);
            _flags[index] &= ~mask;
        }

        template<uint32_t... Bits>
        SYN_INLINE bool IsBitSetImpl(DenseIndex index) const {
            constexpr uint32_t mask = ((1 << Bits) | ...);
            return (_flags[index] & mask) == mask;
        }

        SYN_INLINE void ResetAllBitsImpl(DenseIndex index)
        {
            _flags[index] = 0;
        }

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSetImpl() const {
            constexpr uint32_t mask = ((1 << Bits) | ...);
            return (_state.load(std::memory_order_relaxed) & mask) == mask;
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBitImpl() {
            constexpr uint32_t mask = ((1 << Bits) | ...);
            _state.fetch_and(~mask, std::memory_order_relaxed);
        }

        SYN_INLINE void ResetAllStateBitsImpl()
        {
            _state.store(0, std::memory_order_relaxed);
        }
    protected:
        std::vector<uint32_t> _flags;
        std::atomic<uint32_t> _state{ 0 };
    };
}