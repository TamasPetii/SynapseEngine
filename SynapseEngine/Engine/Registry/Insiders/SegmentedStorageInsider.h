#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include <cstddef>

namespace Syn
{
    template<typename T, typename F> class SegmentedStorageImpl;

    class SYN_API SegmentedStorageInsider
    {
    public:
        static Passkey<SegmentedStorageInsider> GetKey() { return {}; }

        template<typename T, typename F>
        static size_t& GetStaticEnd(SegmentedStorageImpl<T, F>& ss, Passkey<SegmentedStorageInsider>) {
            return ss._staticEnd;
        }

        template<typename T, typename F>
        static size_t& GetDynamicEnd(SegmentedStorageImpl<T, F>& ss, Passkey<SegmentedStorageInsider>) {
            return ss._dynamicEnd;
        }
    };
}