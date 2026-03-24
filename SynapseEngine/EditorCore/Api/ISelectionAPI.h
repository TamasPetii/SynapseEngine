#pragma once
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ISelectionAPI {
    public:
        virtual ~ISelectionAPI() = default;
        virtual EntityID GetSelectedEntity() const = 0;
        virtual void SetSelectedEntity(EntityID entity) = 0;
    };
}