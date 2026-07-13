#pragma once
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "Engine/Engine.h"
#include "Editor/Manager/GuiTextureManager.h"

#include "EditorCore/Api/IApi.h" 
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
#include "EditorCore/Api/IPointLightApi.h"
#include "EditorCore/Api/ISpotLightApi.h"
#include "EditorCore/Api/ITextureApi.h"
#include "EditorCore/Api/IModelApi.h"
#include "EditorCore/Api/ICameraApi.h"
#include "EditorCore/Api/IBoxColliderApi.h"
#include "EditorCore/Api/ISphereColliderApi.h"
#include "EditorCore/Api/ICapsuleColliderApi.h"
#include "EditorCore/Api/IConvexColliderApi.h"
#include "EditorCore/Api/IMeshColliderApi.h"
#include "EditorCore/Api/IRigidBodyApi.h"
#include "EditorCore/Api/IModelComponentApi.h"
#include "EditorCore/Api/IAnimationApi.h"
#include "EditorCore/Api/IMaterialOverrideApi.h"
#include "EditorCore/Api/IPreviewApi.h"

namespace Syn {
    class EditorContext {
    public:
        EditorContext(Engine* engine, GuiTextureManager* textureManager);

        template <typename T>
        T* GetApi() const {
            auto it = _apis.find(std::type_index(typeid(T)));

            if (it != _apis.end()) {
                return static_cast<T*>(it->second.get());
            }

            return nullptr;
        }

    private:
        template <typename Interface, typename Implementation, typename... Args>
        void RegisterApi(Args&&... args) {
            _apis[std::type_index(typeid(Interface))] = std::make_unique<Implementation>(std::forward<Args>(args)...);
        }

        std::unordered_map<std::type_index, std::unique_ptr<IApi>> _apis;
    };
}