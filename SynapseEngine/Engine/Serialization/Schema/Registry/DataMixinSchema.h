#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h" 

#include "Engine/Registry/Pool/Storage/Mixin/Data/DataMixin.h"
#include "Engine/Registry/Insiders/DataMixinInsider.h"

namespace Syn
{
    template<typename T>
    struct SYN_API Schema<DataMixin<T>> {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            if constexpr (!std::is_void_v<T>) 
            {
                auto& v = const_cast<std::remove_const_t<U>&>(val);
                auto& data = DataMixinInsider::GetData(v, DataMixinInsider::GetKey());

                if constexpr (!std::is_trivially_copyable_v<T>) {
                    ar.Property("data", data);
                }
                else {
                    if (ar.IsBinary())
                    {
                        BlitVector<T> blitData{ data };
                        ar.Property("data", blitData);
                    }
                    else
                    {
                        ar.Property("data", data);
                    }
                }
            }
        }
    };
}