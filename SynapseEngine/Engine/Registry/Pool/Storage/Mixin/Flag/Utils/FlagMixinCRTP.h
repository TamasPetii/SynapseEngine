#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"

namespace Syn
{
    template<typename Derived>
    struct FlagMixinCRTP
    {
        SYN_INLINE void PushFlag() {
            static_cast<Derived*>(this)->PushFlagImpl();
        }

        SYN_INLINE void PopFlag() {
            static_cast<Derived*>(this)->PopFlagImpl();
        }

        SYN_INLINE void SwapFlag(DenseIndex a, DenseIndex b) {
            static_cast<Derived*>(this)->SwapFlagImpl(a, b);
        }

        SYN_INLINE void FlagIndexChanged(DenseIndex index) {
            static_cast<Derived*>(this)->FlagIndexChangedImpl(index);
        }

        SYN_INLINE void ClearFlags() {
            static_cast<Derived*>(this)->ClearFlagsImpl();
        }

        template<uint32_t... Bits>
        SYN_INLINE bool SetBit(DenseIndex index) {
            return static_cast<Derived*>(this)->template SetBitImpl<Bits...>(index);
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetBit(DenseIndex index) {
            static_cast<Derived*>(this)->template ResetBitImpl<Bits...>(index);
        }

        template<uint32_t... Bits>
        SYN_INLINE bool IsBitSet(DenseIndex index) const {
            return static_cast<const Derived*>(this)->template IsBitSetImpl<Bits...>(index);
        }

        SYN_INLINE void ResetAllBits(DenseIndex index) {
            static_cast<Derived*>(this)->ResetAllBitsImpl(index);
        }

        template<uint32_t... Bits>
        SYN_INLINE bool IsStateBitSet() const {
            return static_cast<const Derived*>(this)->template IsStateBitSetImpl<Bits...>();
        }

        template<uint32_t... Bits>
        SYN_INLINE void ResetStateBit() {
            static_cast<Derived*>(this)->template ResetStateBitImpl<Bits...>();
        }

        SYN_INLINE void ResetAllStateBits() {
            static_cast<Derived*>(this)->ResetAllStateBitsImpl();
        }
    };
}