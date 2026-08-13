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

#include "EditorContext.h"
#include "Impl/DirectionLightApiImpl.h"
#include "Impl/TagApiImpl.h"
#include "Impl/TransformApiImpl.h"
#include "Impl/FileSystemApiImpl.h"
#include "Impl/HierarchyApiImpl.h"
#include "Impl/LoggerApiImpl.h"
#include "Impl/MaterialApiImpl.h"
#include "Impl/RenderApiImpl.h"
#include "Impl/SceneApiImpl.h"
#include "Impl/SettingsApiImpl.h"
#include "Impl/SelectionApiImpl.h"
#include "Impl/PointLightApiImpl.h"
#include "Impl/SpotLightApiImpl.h"
#include "Impl/TextureApiImpl.h"
#include "Impl/ModelApiImpl.h"
#include "Impl/CameraApiImpl.h"
#include "Impl/BoxColliderApiImpl.h"
#include "Impl/SphereColliderApiImpl.h"
#include "Impl/CapsuleColliderApiImpl.h"
#include "Impl/ConvexColliderApiImpl.h"
#include "Impl/MeshColliderApiImpl.h"
#include "Impl/RigidBodyApiImpl.h"
#include "Impl/ModelComponentApiImpl.h"
#include "Impl/AnimationCompApiImpl.h"
#include "Impl/MaterialOverrideApiImpl.h"
#include "Impl/PreviewApiImpl.h"
#include "Impl/PipelineOverrideApiImpl.h"
#include "Impl/AudioSourceApiImpl.h"
#include "Impl/AudioListenerApiImpl.h"
#include "Impl/AnimationApiImpl.h"
#include "Impl/AudioApiImpl.h"

namespace Syn {
    EditorContext::EditorContext(Engine* engine, GuiTextureManager* textureManager) {
        SceneManager* sm = engine->GetSceneManager();

        RegisterApi<ISelectionApi, SelectionApiImpl>(sm);
        RegisterApi<ITagApi, TagApiImpl>(sm);
        RegisterApi<ITransformApi, TransformApiImpl>(sm);
        RegisterApi<IDirectionLightApi, DirectionLightApiImpl>(sm);
        RegisterApi<IFileSystemApi, FileSystemApiImpl>();
        RegisterApi<IHierarchyApi, HierarchyApiImpl>(sm, engine->GetModelManager());
        RegisterApi<ILoggerApi, LoggerApiImpl>(engine);
        RegisterApi<IMaterialApi, MaterialApiImpl>(engine->GetMaterialManager(), sm);
        RegisterApi<IRenderApi, RenderApiImpl>(engine, textureManager, sm);
        RegisterApi<ISceneApi, SceneApiImpl>(sm);
        RegisterApi<ISettingsApi, SettingsApiImpl>(sm);
        RegisterApi<IPointLightApi, PointLightApiImpl>(sm);
        RegisterApi<ISpotLightApi, SpotLightApiImpl>(sm);
        RegisterApi<ITextureApi, TextureApiImpl>(engine->GetImageManager(), textureManager);
        RegisterApi<IModelApi, ModelApiImpl>(engine->GetModelManager(), sm);
        RegisterApi<ICameraApi, CameraApiImpl>(sm);
        RegisterApi<IBoxColliderApi, BoxColliderApiImpl>(sm);
        RegisterApi<ISphereColliderApi, SphereColliderApiImpl>(sm);
        RegisterApi<ICapsuleColliderApi, CapsuleColliderApiImpl>(sm);
        RegisterApi<IConvexColliderApi, ConvexColliderApiImpl>(sm);
        RegisterApi<IMeshColliderApi, MeshColliderApiImpl>(sm);
        RegisterApi<IRigidBodyApi, RigidBodyApiImpl>(sm);
        RegisterApi<IModelComponentApi, ModelComponentApiImpl>(sm);
        RegisterApi<IAnimationCompApi, AnimationCompApiImpl>(sm);
        RegisterApi<IMaterialOverrideApi, MaterialOverrideApiImpl>(sm);
        RegisterApi<IPipelineOverrideApi, PipelineOverrideApiImpl>(sm);
        RegisterApi<IPreviewApi, PreviewApiImpl>(engine->GetPreviewManager(), textureManager, engine->GetImageManager());
        RegisterApi<IAudioSourceApi, AudioSourceApiImpl>(sm);
        RegisterApi<IAudioListenerApi, AudioListenerApiImpl>(sm);
        RegisterApi<IAnimationApi, AnimationApiImpl>(engine->GetAnimationManager(), sm);
        RegisterApi<IAudioApi, AudioApiImpl>(engine->GetAudioManager());
    }
}