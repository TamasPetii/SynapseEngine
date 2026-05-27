#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include "Engine/Registry/Entity.h"
#include <vector>

namespace Syn
{
    template<typename T> struct DataMixin;

    class SYN_API DataMixinInsider
    {
    public:
        static Passkey<DataMixinInsider> GetKey() { return {}; }

        template<typename T>
        static std::vector<T>& GetData(DataMixin<T>& mixin, Passkey<DataMixinInsider>) {
            return mixin._data;
        }
    };
}