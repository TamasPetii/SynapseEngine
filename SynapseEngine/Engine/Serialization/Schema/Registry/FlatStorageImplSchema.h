#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Registry/Pool/Storage/Core/FlatStorageImpl.h" 
#include "StorageBackendSchema.h" 

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
    struct Schema<FlatStorageImpl<T, FlagMixinPolicy>>
    {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);

            Schema<StorageBackend<T, FlagMixinPolicy>>::Invoke(ar, "backend", v);
        }
    };
}