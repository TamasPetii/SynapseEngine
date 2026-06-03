#pragma once
#include "ISelectionAPI.h"
#include "ITransformAPI.h"
#include "IRenderAPI.h"
#include "ISettingsApi.h"
#include "ISceneAPI.h"
#include "IMaterialAPI.h"
#include "IFileSystemAPI.h"
#include "IHierarchyAPI.h"
#include "ITagAPI.h"

namespace Syn {
    class IEditorAPI :
        public ISelectionAPI,
        public ITransformAPI,
        public IRenderAPI,
        public ISettingsAPI,
        public ISceneAPI,
        public IMaterialAPI,
        public IFileSystemAPI,
        public IHierarchyAPI,
        public ITagAPI
    {
    public:
        virtual ~IEditorAPI() = default;
    };
}