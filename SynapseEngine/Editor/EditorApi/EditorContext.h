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
        IPointLightApi* GetPointLightApi() const { return _pointLightApi.get(); }
        ISpotLightApi* GetSpotLightApi() const { return _spotLightApi.get(); }
        ITextureApi* GetTextureApi() const { return _textureApi.get(); }
        IModelApi* GetModelApi() const { return _modelApi.get(); }
        ICameraApi* GetCameraApi() const { return _cameraApi.get(); }
        IBoxColliderApi* GetBoxColliderApi() const { return _boxColliderApi.get(); }
        ISphereColliderApi* GetSphereColliderApi() const { return _sphereColliderApi.get(); }
        ICapsuleColliderApi* GetCapsuleColliderApi() const { return _capsuleColliderApi.get(); }
        IConvexColliderApi* GetConvexColliderApi() const { return _convexColliderApi.get(); }
        IMeshColliderApi* GetMeshColliderApi() const { return _meshColliderApi.get(); }
        IRigidBodyApi* GetRigidBodyApi() const { return _rigidBodyApi.get(); }
		IModelComponentApi* GetModelComponentApi() const { return _modelComponentApi.get(); }
		IAnimationApi* GetAnimationApi() const { return _animationApi.get(); }
        IMaterialOverrideApi* GetMaterialOverrideApi() { return _materialOverrideApi.get(); }
    private:
        std::unique_ptr<ISelectionApi> _selectionApi;
        std::unique_ptr<ITagApi> _tagApi;
        std::unique_ptr<ITransformApi> _transformApi;
        std::unique_ptr<IFileSystemApi> _fileSystemApi;
        std::unique_ptr<IHierarchyApi> _hierarchyApi;
        std::unique_ptr<ILoggerApi> _loggerApi;
        std::unique_ptr<IMaterialApi> _materialApi;
        std::unique_ptr<IRenderApi> _renderApi;
        std::unique_ptr<ISceneApi> _sceneApi;
        std::unique_ptr<ISettingsApi> _settingsApi;
        std::unique_ptr<IDirectionLightApi> _directionLightApi;
        std::unique_ptr<IPointLightApi> _pointLightApi;
        std::unique_ptr<ISpotLightApi> _spotLightApi;
        std::unique_ptr<ITextureApi> _textureApi;
        std::unique_ptr<IModelApi> _modelApi;
        std::unique_ptr<ICameraApi> _cameraApi;
        std::unique_ptr<IBoxColliderApi> _boxColliderApi;
        std::unique_ptr<ISphereColliderApi> _sphereColliderApi;
        std::unique_ptr<ICapsuleColliderApi> _capsuleColliderApi;
        std::unique_ptr<IConvexColliderApi> _convexColliderApi;
        std::unique_ptr<IMeshColliderApi> _meshColliderApi;
        std::unique_ptr<IRigidBodyApi> _rigidBodyApi;
		std::unique_ptr<IModelComponentApi> _modelComponentApi;
		std::unique_ptr<IAnimationApi> _animationApi;
        std::unique_ptr<IMaterialOverrideApi> _materialOverrideApi;
    };
}