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

#include "TransformModelLinkSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Core/TagSystem.h"
#include <print>

namespace Syn
{
    std::vector<TypeID> TransformModelLinkSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID
        };
    }

    std::vector<TypeID> TransformModelLinkSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TransformModelLinkSystem>::ID };
    }

    void TransformModelLinkSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto [transformPool, modelPool] = registry->GetPools<TransformComponent, ModelComponent>();

        if (!transformPool || !modelPool) {
            return;
        }

        auto componentBufferManager = scene->GetComponentBufferManager();
        auto linkBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TransformModelLinkData, frameIndex);

        if (!linkBuffer.buffer) {
            return;
        }

        if (_gpuLinkVersions.size() <= frameIndex)
            _gpuLinkVersions.resize(frameIndex + 1, 0);

        // We update the links if:
        // 1. A transform moved or was added (Transform INDEX_CHANGED_BIT or CHANGED_BIT)
        // 2. A model was changed or added (Model CHANGED_BIT)
        // 3. The static hierarchy was rebuilt (reordering occurred)
        bool transformDirty = transformPool->IsStateBitSet<INDEX_CHANGED_BIT>();
        bool modelDirty = modelPool->IsStateBitSet<CHANGED_BIT>() || modelPool->IsStateBitSet<INDEX_CHANGED_BIT>();
        bool staticDirty = !transformPool->GetStorage().GetDirtyStatics().empty();

        bool indexChanged = transformPool->IsStateBitSet<INDEX_CHANGED_BIT>();
        bool forceUpload = this->ShouldForceUpload() || indexChanged;

        if (transformDirty || modelDirty || staticDirty || forceUpload) {
            _globalLinkVersion++;
        }

        if (_gpuLinkVersions[frameIndex] == _globalLinkVersion) {
            return;
        }

        bool isCatchingUp = (_gpuLinkVersions[frameIndex] < _globalLinkVersion);
        bool effectiveForceUpload = forceUpload || isCatchingUp;

        _gpuLinkVersions[frameIndex] = _globalLinkVersion;

        auto linkBufferHandler = static_cast<TransformModelLinkGPU*>(linkBuffer.buffer->Map());

        auto processLink = [transformPool, modelPool, linkBuffer, linkBufferHandler, effectiveForceUpload](EntityID entity)
            {
                auto transformIndex = transformPool->GetMapping().Get(entity);
                auto& transformComp = transformPool->Get(entity);

                if (effectiveForceUpload || linkBuffer.versions[transformIndex] != transformComp.version)
                {
                    uint32_t modelIdx = NULL_INDEX;
                    if (modelPool->Has(entity))
                        modelIdx = modelPool->GetMapping().Get(entity);

                    linkBufferHandler[transformIndex] = { entity, modelIdx };
                    linkBuffer.versions[transformIndex] = transformComp.version;
                }
            };

        auto staticEntities = transformPool->GetStorage().GetStaticEntities();
        auto dynamicEntities = transformPool->GetStorage().GetDynamicEntities();
        auto streamEntities = transformPool->GetStorage().GetStreamEntities();

        if (!streamEntities.empty())
            subflow.for_each(streamEntities.begin(), streamEntities.end(), processLink);

        if (effectiveForceUpload || transformDirty || modelDirty)
            subflow.for_each(dynamicEntities.begin(), dynamicEntities.end(), processLink);

        if (effectiveForceUpload || staticDirty || modelDirty)
            subflow.for_each(staticEntities.begin(), staticEntities.end(), processLink);
    }
}