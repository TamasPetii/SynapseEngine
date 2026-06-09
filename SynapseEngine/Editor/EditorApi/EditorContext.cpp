#include "EditorContext.h"
#include "Impl/DirectionLightApiImpl.h"
#include "Impl/TagApiImpl.h"
#include "Impl/TransformApiImpl.h"
#include "Impl/DirectionLightApiImpl.h"
#include "Impl/FileSystemApiImpl.h"
#include "Impl/HierarchyApiImpl.h"
#include "Impl/LoggerApiImpl.h"
#include "Impl/MaterialApiImpl.h"
#include "Impl/RenderApiImpl.h"
#include "Impl/SceneApiImpl.h"
#include "Impl/SettingsApiImpl.h"
#include "Impl/SelectionApiImpl.h"

namespace Syn {
    EditorContext::EditorContext(Engine* engine, GuiTextureManager* textureManager) {
        SceneManager* sm = engine->GetSceneManager();

        _selectionApi = std::make_unique<SelectionApiImpl>(sm);
        _tagApi = std::make_unique<TagApiImpl>(sm);
        _transformApi = std::make_unique<TransformApiImpl>(sm);
        _directionLightApi = std::make_unique<DirectionLightApiImpl>(sm);
        _fileSystemApi = std::make_unique<FileSystemApiImpl>();
        _hierarchyApi = std::make_unique<HierarchyApiImpl>(sm);
        _loggerApi = std::make_unique<LoggerApiImpl>(engine);
        _materialApi = std::make_unique<MaterialApiImpl>(engine);
        _renderApi = std::make_unique<RenderApiImpl>(engine, textureManager, sm);
        _sceneApi = std::make_unique<SceneApiImpl>(sm);
        _settingsApi = std::make_unique<SettingsApiImpl>(sm);
    }

    EditorContext::~EditorContext() = default;
}