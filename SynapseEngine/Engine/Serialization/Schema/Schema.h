#pragma once
#include "Engine/SynApi.h"
#include "Engine/Serialization/Archive/Utils.h"
#include <utility>
#include <type_traits>

namespace Syn
{
    template <typename T>
    struct ComponentName;

    #define SYN_REGISTER_COMPONENT(Type, Name) \
        template <> struct ComponentName<Type> { \
            static constexpr const char* value = Name; \
        }

    template <typename T, typename Enable = void>
    struct Schema {
        static constexpr bool exists = false;
    };

    template <typename T, typename Archive = void>
    concept has_schema = Schema<std::remove_cvref_t<T>>::exists;
}