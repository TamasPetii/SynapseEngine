#pragma once
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ISelectionApi {
    public:
        virtual ~ISelectionApi() = default;
        virtual EntityID GetSelectedEntity() const = 0;
        virtual void SetSelectedEntity(EntityID entity) = 0;
    };
}