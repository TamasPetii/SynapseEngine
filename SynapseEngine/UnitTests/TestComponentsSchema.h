#pragma once

#include "TestComponents.h"
#include "Engine/Serialization/Schema/Schema.h"

namespace Syn
{
    SYN_REGISTER_COMPONENT(TagEnemy, "TagEnemy");

    template <>
    struct Schema<TagEnemy> {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) {
            ScopedArchiveObject obj(ar, name);
        }
    };

    SYN_REGISTER_COMPONENT(Velocity, "Velocity");

    template <>
    struct Schema<Velocity> {
        static constexpr bool exists = true;
        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) {
            ScopedArchiveObject obj(ar, name);

            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("dx", v.dx);
            ar.Property("dy", v.dy);
        }
    };

    SYN_REGISTER_COMPONENT(Health, "Health");

    template <>
    struct Schema<Health> {
        static constexpr bool exists = true;
        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val) {
            ScopedArchiveObject obj(ar, name);
            auto& v = const_cast<std::remove_const_t<U>&>(val);
            ar.Property("hp", v.hp);
            ar.Property("maxHp", v.maxHp);
        }
    };
}