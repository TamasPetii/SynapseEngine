#pragma once
#include <type_traits>
#include "MappingCRTP.h"

namespace Syn
{
    template<typename T>
    concept MappingConstraint = std::is_base_of_v<MappingCRTP<T>, T>;
}