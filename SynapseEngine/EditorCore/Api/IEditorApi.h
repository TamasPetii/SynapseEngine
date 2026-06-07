#pragma once
#include "ISelectionApi.h"
#include "ITransformApi.h"
#include "IRenderApi.h"
#include "ISettingsApi.h"
#include "ISceneApi.h"
#include "IMaterialApi.h"
#include "IFileSystemApi.h"
#include "IHierarchyApi.h"
#include "ITagApi.h"
#include "ILoggerApi.h"

namespace Syn {
    class IEditorApi :
        public ISelectionApi,
        public ITransformApi,
        public IRenderApi,
        public ISettingsApi,
        public ISceneApi,
        public IMaterialApi,
        public IFileSystemApi,
        public IHierarchyApi,
        public ITagApi,
		public ILoggerApi
    {
    public:
        virtual ~IEditorApi() = default;
    };
}