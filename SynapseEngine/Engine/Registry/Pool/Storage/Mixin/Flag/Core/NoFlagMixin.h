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

        template<uint32_t... Bits> SYN_INLINE void SetStateBitSetImpl() const { }

        template<uint32_t... Bits> SYN_INLINE bool IsStateBitSet() const { return false; }
        template<uint32_t... Bits> SYN_INLINE void ResetStateBit() {}
        SYN_INLINE void ResetAllStateBits() {}

        SYN_INLINE void InitializeFlagsImpl(size_t) {}
    };
}