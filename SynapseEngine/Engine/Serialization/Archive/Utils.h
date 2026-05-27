#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    template <typename Archive>
    struct ScopedArchiveObject {
        ScopedArchiveObject(Archive& ar, const char* name) : _ar(ar) { _ar.EnterObject(name); }
        ~ScopedArchiveObject() { _ar.LeaveObject(); }
        Archive& _ar;
    };

    template <typename Archive>
    struct ScopedArchiveArray {
        ScopedArchiveArray(Archive& ar, const char* name, size_t size) : _ar(ar) { _ar.EnterArray(name, size); }
        ~ScopedArchiveArray() { _ar.LeaveArray(); }
        Archive& _ar;
    };
}