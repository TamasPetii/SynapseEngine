#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include "Engine/Registry/Entity.h"
#include "Engine/Registry/BitFlag.h"
#include <vector>

namespace Syn
{
    template<bool Enable>
    struct FlagMixin;

    template<>
    struct FlagMixin<true>
    {
    public:
        void PushFlag();
        void PopFlag();
        void SwapFlag(DenseIndex a, DenseIndex b);
        void FlagIndexChanged(DenseIndex index);
        void ClearFlags();

        BitsetFlag      GetState() const;
        void            ResetState();

        BitsetFlag& GetFlags(DenseIndex index);
        const BitsetFlag& GetFlags(DenseIndex index) const;

        template<uint32_t... Bits>
        void SetBit(DenseIndex index);

        template<uint32_t... Bits>
        void ResetBit(DenseIndex index);

        template<uint32_t... Bits>
        bool IsBitSet(DenseIndex index) const;
    protected:
        std::vector<BitsetFlag> _flags;
        BitsetFlag              _state;
    };

    template<>
    struct FlagMixin<false>
    {
        SYN_INLINE void PushFlag() {}
        SYN_INLINE void PopFlag() {}
        SYN_INLINE void SwapFlag(DenseIndex, DenseIndex) {}
        SYN_INLINE void FlagIndexChanged(DenseIndex) {}
        SYN_INLINE void ClearFlags() {}
        SYN_INLINE BitsetFlag GetState() const { return {}; }
        SYN_INLINE void       ResetState() {}
    };
}

namespace Syn
{
    SYN_INLINE void FlagMixin<true>::PushFlag()
    {
        BitsetFlag newFlag;

        newFlag.set(REGENERATE_BIT);
        newFlag.set(UPDATE_BIT);
        newFlag.set(INDEX_CHANGED_BIT);

        _flags.push_back(newFlag);

        _state.set(REGENERATE_BIT);
        _state.set(UPDATE_BIT);
        _state.set(INDEX_CHANGED_BIT);
    }

    SYN_INLINE void FlagMixin<true>::PopFlag()
    {
        SYN_ASSERT(!_flags.empty(), "Attempting to pop from empty flag vector");
        _flags.pop_back();
    }

    SYN_INLINE void FlagMixin<true>::SwapFlag(DenseIndex a, DenseIndex b)
    {
        SYN_ASSERT(a < _flags.size() && b < _flags.size(), "Flag index out of bounds");
        std::swap(_flags[a], _flags[b]);
    }

    SYN_INLINE void FlagMixin<true>::FlagIndexChanged(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        _flags[index].set(INDEX_CHANGED_BIT);
        _state.set(INDEX_CHANGED_BIT);
    }

    SYN_INLINE void FlagMixin<true>::ClearFlags()
    {
        _flags.clear();
        _state.reset();
    }

    SYN_INLINE BitsetFlag& FlagMixin<true>::GetFlags(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        return _flags[index];
    }

    SYN_INLINE const BitsetFlag& FlagMixin<true>::GetFlags(DenseIndex index) const
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        return _flags[index];
    }

    template<uint32_t... Bits>
    SYN_INLINE void FlagMixin<true>::SetBit(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        (_flags[index].set(Bits), ...);
        (_state.set(Bits), ...);
    }

    template<uint32_t... Bits>
    SYN_INLINE void FlagMixin<true>::ResetBit(DenseIndex index)
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        (_flags[index].reset(Bits), ...);
    }

    template<uint32_t... Bits>
    SYN_INLINE bool FlagMixin<true>::IsBitSet(DenseIndex index) const
    {
        SYN_ASSERT(index < _flags.size(), "Flag index out of bounds");
        return (_flags[index].test(Bits) && ...);
    }

    SYN_INLINE BitsetFlag FlagMixin<true>::GetState() const
    {
        return _state;
    }

    SYN_INLINE void FlagMixin<true>::ResetState()
    {
        _state.reset();
    }
}