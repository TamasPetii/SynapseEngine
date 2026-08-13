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
        SYN_INLINE void SetStateBitSet() const {
            static_cast<const Derived*>(this)->template SetStateBitSetImpl<Bits...>();
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

        SYN_INLINE void InitializeFlags(size_t count) {
            static_cast<Derived*>(this)->InitializeFlagsImpl(count);
        }
    };
}