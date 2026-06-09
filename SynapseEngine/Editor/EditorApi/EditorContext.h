#pragma once
#include <memory>
#include "Engine/Engine.h"
#include "Editor/Manager/GuiTextureManager.h"

#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITagApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"
#include "EditorCore/Api/IFileSystemApi.h"
#include "EditorCore/Api/IHierarchyApi.h"
#include "EditorCore/Api/ILoggerApi.h"
#include "EditorCore/Api/IMaterialApi.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ISceneApi.h"
#include "EditorCore/Api/ISettingsApi.h"

namespace Syn {
    class EditorContext {
    public:
        EditorContext(Engine* engine, GuiTextureManager* textureManager);
        ~EditorContext();

        ISelectionApi* GetSelectionApi() const { return _selectionApi.get(); }
        ITagApi* GetTagApi() const { return _tagApi.get(); }
        ITransformApi* GetTransformApi() const { return _transformApi.get(); }
        IDirectionLightApi* GetDirectionLightApi() const { return _directionLightApi.get(); }
        IFileSystemApi* GetFileSystemApi() const { return _fileSystemApi.get(); }
        IHierarchyApi* GetHierarchyApi() const { return _hierarchyApi.get(); }
        ILoggerApi* GetLoggerApi() const { return _loggerApi.get(); }
        IMaterialApi* GetMaterialApi() const { return _materialApi.get(); }
        IRenderApi* GetRenderApi() const { return _renderApi.get(); }
        ISceneApi* GetSceneApi() const { return _sceneApi.get(); }
        ISettingsApi* GetSettingsApi() const { return _settingsApi.get(); }

    private:
        std::unique_ptr<ISelectionApi> _selectionApi;
        std::unique_ptr<ITagApi> _tagApi;
        std::unique_ptr<ITransformApi> _transformApi;
        std::unique_ptr<IDirectionLightApi> _directionLightApi;
        std::unique_ptr<IFileSystemApi> _fileSystemApi;
        std::unique_ptr<IHierarchyApi> _hierarchyApi;
        std::unique_ptr<ILoggerApi> _loggerApi;
        std::unique_ptr<IMaterialApi> _materialApi;
        std::unique_ptr<IRenderApi> _renderApi;
        std::unique_ptr<ISceneApi> _sceneApi;
        std::unique_ptr<ISettingsApi> _settingsApi;
    };
}