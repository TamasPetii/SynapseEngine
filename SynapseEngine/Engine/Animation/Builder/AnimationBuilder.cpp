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

#include "AnimationBuilder.h"
#include "Engine/Animation/Source/File/FileAnimationSource.h"
#include "Engine/Serialization/Schema/Animation/GpuBatchedAnimationSchema.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/EnginePaths.h"

namespace Syn
{
    AnimationBuilder::AnimationBuilder(
        std::unique_ptr<IAnimationLoaderRegistry> registry,
        std::unique_ptr<IAnimationProcessorPipeline> pipeline,
        std::unique_ptr<IGpuAnimationConverter> converter,
        std::unique_ptr<IAnimationCooker> cooker) :
        _registry(std::move(registry)),
        _cooker(std::move(cooker)),
        _pipeline(std::move(pipeline)),
        _converter(std::move(converter))
    {}

    void AnimationBuilder::RegisterLoader(std::shared_ptr<IAnimationLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void AnimationBuilder::RegisterProcessor(std::unique_ptr<IAnimationProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Animation> AnimationBuilder::BuildFromFile(const std::string& filePath, const CpuModelData& baseModel)
    {
        std::filesystem::path srcPath(filePath);

        std::filesystem::path saveDir = EnginePaths::GetAnimationsCacheDir();
        std::filesystem::path cachePath = saveDir / srcPath.filename();
        cachePath.replace_extension(".synanim");

        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directories(saveDir);
        }

        auto animation = std::make_shared<Animation>();
        animation->transientCpuData = std::make_unique<CookedAnimation>();
        animation->transientGpuData = std::make_unique<GpuBatchedAnimation>();

        auto serializer = ServiceLocator::Get<Serializer>();

        bool useCache = false;
        if (std::filesystem::exists(cachePath) && std::filesystem::exists(srcPath)) {
            if (std::filesystem::last_write_time(cachePath) >= std::filesystem::last_write_time(srcPath)) {
                useCache = true;
            }
        }

        if (useCache && serializer) {
            if (serializer->LoadFromFile(cachePath, *(animation->transientGpuData))) {
                Info("Loaded {} from binary cache.", srcPath.filename().string());
                return animation;
            }
            Warning("Cache corrupted for {}, rebuilding.", srcPath.filename().string());
        }

        Info("Cooking {} from source...", srcPath.filename().string());

        std::string ext = std::filesystem::path(filePath).extension().string();
        IAnimationLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileAnimationSource source(filePath, loader);
        auto generatedAnim = BuildFromSource(source, baseModel);

        if (!generatedAnim || !generatedAnim->transientGpuData)
            return nullptr;

        if (serializer) {
            serializer->SaveToFile(cachePath, *(generatedAnim->transientGpuData));
        }

        return generatedAnim;
    }

    std::shared_ptr<Animation> AnimationBuilder::BuildFromSource(IAnimationSource& source, const CpuModelData& baseModel)
    {
        auto rawAnimOpt = source.Produce();

        if (!rawAnimOpt)
            return nullptr;

        auto animation = std::make_shared<Animation>();
        animation->transientCpuData = std::make_unique<CookedAnimation>();
        animation->transientGpuData = std::make_unique<GpuBatchedAnimation>();

        *(animation->transientCpuData) = _cooker->Cook(std::move(rawAnimOpt).value());
        _pipeline->Run(*(animation->transientCpuData), baseModel);
        *(animation->transientGpuData) = _converter->Convert(*(animation->transientCpuData), baseModel);

        return animation;
    }
}