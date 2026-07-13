#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include <cstdint>

namespace Syn {
    struct ComponentState {
        bool hasSelection = false;
        EntityID activeEntityId = NULL_ENTITY;
    };
}