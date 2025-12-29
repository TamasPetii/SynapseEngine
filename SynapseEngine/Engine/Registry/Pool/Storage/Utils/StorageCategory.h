#pragma once
#include "Engine/SynMacro.h"
#include <ostream>

enum class StorageCategory {
    Static,
    Dynamic,
    Stream
};

SYN_INLINE std::ostream& operator<<(std::ostream& os, StorageCategory cat) {
    switch (cat) {
    case StorageCategory::Static:  return os << "Static";
    case StorageCategory::Dynamic: return os << "Dynamic";
    case StorageCategory::Stream:  return os << "Stream";
    default: return os << "Unknown(" << static_cast<int>(cat) << ")";
    }
}