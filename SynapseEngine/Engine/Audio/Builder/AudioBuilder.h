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
#include "Engine/Audio/Loader/IAudioLoaderRegistry.h"
#include "Engine/Audio/Processor/IAudioProcessorPipeline.h"
#include "Engine/Audio/Converter/IAudioCooker.h"
#include "Engine/Audio/Source/IAudioSource.h"
#include "Engine/Audio/Converter/ICpuAudioExtractor.h"
#include "Engine/Audio/Data/Sound.h"

#include <memory>
#include <string>
#include <filesystem>

namespace Syn
{
    class SYN_API AudioBuilder
    {
    public:
        AudioBuilder(
            std::unique_ptr<IAudioLoaderRegistry> registry,
            std::unique_ptr<IAudioProcessorPipeline> pipeline,
            std::unique_ptr<IAudioCooker> cooker,
            std::unique_ptr<ICpuAudioExtractor> extractor
        );

        AudioBuilder(const AudioBuilder&) = delete;
        AudioBuilder& operator=(const AudioBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IAudioLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IAudioProcessor> processor);

        std::shared_ptr<Sound> BuildFromFile(const std::string& filePath);
        std::shared_ptr<Sound> BuildFromSource(IAudioSource& source);
        IAudioLoader* GetLoaderForExtension(const std::string& ext) const { return _registry->GetLoaderForExtension(ext); }
    private:
        std::unique_ptr<IAudioLoaderRegistry> _registry;
        std::unique_ptr<IAudioProcessorPipeline> _pipeline;
        std::unique_ptr<IAudioCooker> _cooker;
        std::unique_ptr<ICpuAudioExtractor> _extractor;
    };
}