#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::TagComponent, "TagComponent");

    template <>
    struct Schema<TagComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) 
        {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("name", comp.name);
            ar.Property("tag", comp.tag);
        }
    };
}