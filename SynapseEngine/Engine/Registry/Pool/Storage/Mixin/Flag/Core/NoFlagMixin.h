#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "../Utils/FlagMixinCRTP.h"

namespace Syn
{
    struct NoFlagMixin : public FlagMixinCRTP<NoFlagMixin>
    {
        SYN_INLINE void PushFlag() {}
        SYN_INLINE void PopFlag() {}
        SYN_INLINE void SwapFlag(DenseIndex, DenseIndex) {}
        SYN_INLINE void FlagIndexChanged(DenseIndex) {}
        SYN_INLINE void ClearFlags() {}

        template<uint32_t... Bits> SYN_INLINE bool SetBit(DenseIndex) { return false; }
        template<uint32_t... Bits> SYN_INLINE void ResetBit(DenseIndex) {}
        template<uint32_t... Bits> SYN_INLINE bool IsBitSet(DenseIndex) const { return false; }
        SYN_INLINE void ResetAllBits(DenseIndex) {}

        template<uint32_t... Bits> SYN_INLINE bool IsStateBitSet() const { return false; }
        template<uint32_t... Bits> SYN_INLINE void ResetStateBit() {}
        SYN_INLINE void ResetAllStateBits() {}
    };
}