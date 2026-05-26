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

            ar.Property("isReady", val.isReady); //Todo???
            ar.Property("time", val.time);
            ar.Property("speed", val.speed);
            ar.Property("frameIndex", val.frameIndex);
            ar.Property("animationIndex", val.animationIndex);
        }
    };
}