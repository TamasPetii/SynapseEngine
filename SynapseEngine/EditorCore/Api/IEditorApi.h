#pragma once
#include "ISelectionAPI.h"
#include "ITransformAPI.h"
#include "IRenderAPI.h"
#include "ISettingsApi.h"

namespace Syn {
    class IEditorAPI :
        public ISelectionAPI,
        public ITransformAPI,
        public IRenderAPI,
        public ISettingsAPI
    {
    public:
        virtual ~IEditorAPI() = default;
    };
}