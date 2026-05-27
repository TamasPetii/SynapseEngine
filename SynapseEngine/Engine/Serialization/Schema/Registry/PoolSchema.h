#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h" 

#include "Engine/Registry/Pool/Pool.h"

namespace Syn
{
    template<typename T, typename StoragePolicy, typename MappingPolicy>
    struct Schema<Pool<T, StoragePolicy, MappingPolicy>> 
    {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("storage", v.GetStorage());
            ar.Property("mapping", v.GetMapping());
        }
    };
}