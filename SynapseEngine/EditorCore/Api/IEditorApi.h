#pragma once
#include "ISelectionAPI.h"
#include "ITransformAPI.h"
#include "IRenderAPI.h"

namespace Syn {
    class IEditorAPI :
        public ISelectionAPI,
        public ITransformAPI,
        public IRenderAPI
    {
    public:
        virtual ~IEditorAPI() = default;
    };
}