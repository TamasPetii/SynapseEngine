// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <memory>
#include <unordered_map>
#include <typeindex>
#include "Engine/Engine.h"
#include "Editor/Manager/GuiTextureManager.h"

#include "EditorCore/Api/IApi.h" 
#include "EditorCore/Api/IAnimationApi.h" 
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
#include "EditorCore/Api/IAnimationCompApi.h"
#include "EditorCore/Api/IMaterialOverrideApi.h"
#include "EditorCore/Api/IPreviewApi.h"
#include "EditorCore/Api/IPipelineOverrideApi.h"
#include "EditorCore/Api/IAudioSourceApi.h"
#include "EditorCore/Api/IAudioListenerApi.h"
#include "EditorCore/Api/IAudioApi.h"
#include "Engine/Registry/Type/TypeInfo.h"

namespace Syn {
    class EditorContext {
    public:
        EditorContext(Engine* engine, GuiTextureManager* textureManager);

        template <typename T>
        T* GetApi() const 
        {
            const TypeID id = TypeInfo<T>::ID;

            auto it = _apis.find(id);

            if (it != _apis.end()) {
                return static_cast<T*>(it->second.get());
            }

            return nullptr;
        }

    private:
        template <typename Interface, typename Implementation, typename... Args>
        void RegisterApi(Args&&... args) {
            const TypeID id = TypeInfo<Interface>::ID;
            _apis[id] = std::make_unique<Implementation>(std::forward<Args>(args)...);
        }

        std::unordered_map<TypeID, std::unique_ptr<IApi>> _apis;
    };
}