#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/BitFlag.h"
#include "../Utils/FlagMixinCRTP.h"
#include <vector>
#include <bitset>
#include <cstdint>

namespace Syn
{
    template<size_t N = 8>
    struct BitsetFlagMixin : public FlagMixinCRTP<BitsetFlagMixin<N>>
    {
        SYN_INLINE void PushFlagImpl() {
            std::bitset<N> bits;
            bits.set(REGENERATE_BIT);
            bits.set(UPDATE_BIT);
            bits.set(INDEX_CHANGED_BIT);

            _flags.push_back(bits);
            _state |= bits;
        }

        SYN_INLINE void PopFlagImpl() { _flags.pop_back(); }
        SYN_INLINE void SwapFlagImpl(DenseIndex a, DenseIndex b) { std::swap(_flags[a], _flags[b]); }

        SYN_INLINE void FlagIndexChangedImpl(DenseIndex index) {
            _flags[index].set(INDEX_CHANGED_BIT);
            _state.set(INDEX_CHANGED_BIT);
        }

        SYN_INLINE void ClearFlagsImpl() { _flags.clear(); _state.reset(); }

        template<uint32_t... Bits>
        SYN_INLINE bool SetBitImpl(DenseIndex index) {
            bool anySet = ((_flags[index].test(Bits)) || ...);
            ((_flags[index].set(Bits)), ...);
            ((_state.set(Bits)), ...);
            return anySet;
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetBitImpl(DenseIndex index) {
            ((_flags[index].reset(Bits)), ...);
        }

        template<uint32_t... Bits>
        SYN_INLINE bool IsBitSetImpl(DenseIndex index) const {
            return ((_flags[index].test(Bits)) && ...);
        }

        SYN_INLINE void ResetAllBitsImpl(DenseIndex index) { _flags[index].reset(); }

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSetImpl() const {
            return ((_state.test(Bits)) && ...);
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBitImpl() {
            ((_state.reset(Bits)), ...);
        }

        SYN_INLINE void ResetAllStateBitsImpl() { _state.reset(); }

    protected:
        std::vector<std::bitset<N>> _flags;
        std::bitset<N> _state;
    };
}