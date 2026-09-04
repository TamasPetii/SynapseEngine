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

#include "DirectionLightStaticShadowPipeline.h"
#include "DirectionLightStaticShadowCullingPipeline.h"
#include "DirectionLightStaticShadowRenderPipeline.h"
#include "DirectionLightStaticShadowHizPipeline.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Component/Core/TransformComponent.h"

namespace Syn {
    DirectionLightStaticShadowPipeline::DirectionLightStaticShadowPipeline()
        : RenderPipeline("DirectionLightStaticShadowPipeline", PassGroupNames::UndefinedPasses)
    {
        AddPass(std::make_unique<DirectionLightStaticShadowCullingPipeline>());
        AddPass(std::make_unique<DirectionLightStaticShadowRenderPipeline>());
        AddPass(std::make_unique<DirectionLightStaticShadowHizPipeline>());
    }

    bool DirectionLightStaticShadowPipeline::ShouldExecute(const RenderContext& context) const {
        if (context.scene->GetSettings()->culling.directionLightShadowCullingDevice != CullingDeviceType::GPU) {
            return false;
        }

        auto registry = context.scene->GetRegistry();
        auto shadowPool = registry->GetPool<DirectionLightShadowComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto drawData = context.scene->GetSceneDrawData();

        if (!shadowPool || !transformPool) 
            return false;

        uint32_t currentModelVersion = context.scene->GetSystemContext().modelManagerVersion;
        if (drawData->DirectionLightShadow.staticCacheModelVersion.load(std::memory_order_relaxed) != currentModelVersion) 
        {
            drawData->DirectionLightShadow.staticCacheModelVersion.store(currentModelVersion, std::memory_order_relaxed);
            return true;
        }

        uint32_t currentSwapVersion = drawData->DirectionLightShadow.staticModelSwapVersion.load(std::memory_order_relaxed);
        if (drawData->DirectionLightShadow.processedStaticModelSwapVersion.load(std::memory_order_relaxed) != currentSwapVersion)
        {
            drawData->DirectionLightShadow.processedStaticModelSwapVersion.store(currentSwapVersion, std::memory_order_relaxed);
            return true;
        }

        if (!transformPool->GetDirtyStatics().empty())
            return true;

        for (EntityID entity : shadowPool->GetDenseEntities()) 
        {
            if (transformPool->IsBitSet<TRANSFORM_ROT_CHANGED>(entity)) {
                return true;
            }

            const auto& shadowComp = shadowPool->Get(entity);
            for (int i = 0; i < 4; ++i) {
                if (shadowComp.isStaticDirty[i]) {
                    return true;
                }
            }
        }

        return false;
    }
}