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
#include "Engine/SynApi.h"

#include "../Loader/IAnimationLoader.h"
#include "../Loader/IAnimationLoaderRegistry.h"
#include "../Processor/IAnimationProcessor.h"
#include "../Processor/IAnimationProcessorPipeline.h"
#include "../Source/IAnimationSource.h"
#include "../Data/Animation.h"

#include "../Converter/IAnimationCooker.h"
#include "../Converter/IGpuAnimationConverter.h"
#include "../Uploader/IGpuAnimationUploader.h"

#include <memory>
#include <string>

namespace Syn
{
    class SYN_API AnimationBuilder
    {
    public:
        AnimationBuilder(
            std::unique_ptr<IAnimationLoaderRegistry> registry,
            std::unique_ptr<IAnimationProcessorPipeline> pipeline,
            std::unique_ptr<IGpuAnimationConverter> converter,
            std::unique_ptr<IAnimationCooker> cooker
        );

        AnimationBuilder(const AnimationBuilder&) = delete;
        AnimationBuilder& operator=(const AnimationBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IAnimationLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IAnimationProcessor> processor);

        std::shared_ptr<Animation> BuildFromFile(const std::string& filePath, const CpuModelData& baseModel);
        std::shared_ptr<Animation> BuildFromSource(IAnimationSource& source, const CpuModelData& baseModel);
    private:
        std::unique_ptr<IAnimationLoaderRegistry> _registry;
        std::unique_ptr<IAnimationProcessorPipeline> _pipeline;
        std::unique_ptr<IGpuAnimationConverter> _converter;
        std::unique_ptr<IAnimationCooker> _cooker;
    };
}