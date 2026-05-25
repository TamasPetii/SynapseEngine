#pragma once
#include "Engine/SynApi.h"
#include "Engine/Serialization/Archive/Utils.h"
#include <utility>

namespace Syn
{
    template <typename T, typename Enable = void>
    struct SYN_API Schema {
        static constexpr bool exists = false;
    };

    template <typename T, typename Archive = void>
    concept has_schema = Schema<std::remove_cvref_t<T>>::exists;
}