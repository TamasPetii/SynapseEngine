#pragma once
#include <ostream>

enum class StorageCategory {
    Static,
    Dynamic,
    Stream
};

inline std::ostream& operator<<(std::ostream& os, StorageCategory cat) {
    switch (cat) {
    case StorageCategory::Static:  return os << "Static";
    case StorageCategory::Dynamic: return os << "Dynamic";
    case StorageCategory::Stream:  return os << "Stream";
    default: return os << "Unknown(" << static_cast<int>(cat) << ")";
    }
}