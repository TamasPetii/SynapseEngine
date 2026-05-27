#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Component/Rendering/AnimationComponent.h" 

namespace Syn
{
    SYN_REGISTER_COMPONENT(Syn::AnimationComponent, "AnimationComponent");

    template <>
    struct Schema<AnimationComponent> {
        static constexpr bool exists = true;

        template <typename Archive, typename T>
        static void Invoke(Archive& ar, const char* name, T& val) {
            ScopedArchiveObject obj(ar, name);
            auto& comp = const_cast<std::remove_const_t<T>&>(val);

            ar.Property("time", comp.time);
            ar.Property("speed", comp.speed);
            ar.Property("animationIndex", comp.animationIndex);
        }
    };
}