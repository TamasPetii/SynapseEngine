#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h" 

#include "Engine/Registry/Pool/Storage/Core/SegmentedStorageImpl.h"
#include "Engine/Registry/Insiders/SegmentedStorageInsider.h"

namespace Syn
{
    template<typename T, typename FlagMixinPolicy>
    struct Schema<SegmentedStorageImpl<T, FlagMixinPolicy>> 
    {
        static constexpr bool exists = true;

        template<typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) 
        {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);

            auto& sEnd = SegmentedStorageInsider::GetStaticEnd(v, SegmentedStorageInsider::GetKey());
            auto& dEnd = SegmentedStorageInsider::GetDynamicEnd(v, SegmentedStorageInsider::GetKey());

            uint32_t staticEnd = static_cast<uint32_t>(sEnd);
            uint32_t dynamicEnd = static_cast<uint32_t>(dEnd);

            ar.Property("staticEnd", staticEnd);
            ar.Property("dynamicEnd", dynamicEnd);

            if constexpr (std::is_base_of_v<IInputArchive, Archive>) {
                sEnd = staticEnd;
                dEnd = dynamicEnd;
            }

            Schema<StorageBackend<T, FlagMixinPolicy>>::Invoke(ar, "backend", v);
        }
    };
}