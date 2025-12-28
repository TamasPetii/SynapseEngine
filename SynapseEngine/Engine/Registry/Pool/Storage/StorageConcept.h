#pragma once
#include <type_traits>
#include "StorageCRTP.h"

namespace Syn
{
    template<typename T>
    concept StorageConstraint = std::is_base_of_v<StorageCRTP<T, typename T::ValueType>, T>;
}