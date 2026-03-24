#pragma once
#include "ISelectionAPI.h"
#include "ITransformAPI.h"

namespace Syn {
    class IEditorAPI :
        public ISelectionAPI,
        public ITransformAPI
    {
    public:
        virtual ~IEditorAPI() = default;
    };
}