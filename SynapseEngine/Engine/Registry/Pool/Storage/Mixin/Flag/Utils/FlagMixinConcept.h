#pragma once
#include <type_traits>
#include "FlagMixinCRTP.h"

namespace Syn
{
    template<typename T>
    concept FlagMixinConstraint = std::is_base_of_v<FlagMixinCRTP<T>, T>;
}