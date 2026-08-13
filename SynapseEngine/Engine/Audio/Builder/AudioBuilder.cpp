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

#include "AudioBuilder.h"
#include "Engine/Audio/Source/File/FileAudioSource.h"

namespace Syn
{
    AudioBuilder::AudioBuilder(
        std::unique_ptr<IAudioLoaderRegistry> registry,
        std::unique_ptr<IAudioProcessorPipeline> pipeline,
        std::unique_ptr<IAudioCooker> cooker,
        std::unique_ptr<ICpuAudioExtractor> extractor) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _cooker(std::move(cooker)),
        _extractor(std::move(extractor))
    {}

    void AudioBuilder::RegisterLoader(std::shared_ptr<IAudioLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void AudioBuilder::RegisterProcessor(std::unique_ptr<IAudioProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Sound> AudioBuilder::BuildFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IAudioLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileAudioSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::shared_ptr<Sound> AudioBuilder::BuildFromSource(IAudioSource& source)
    {
        auto rawAudioOpt = source.Produce();

        if (!rawAudioOpt)
            return nullptr;

        auto sound = std::make_shared<Sound>();
        sound->transientCpuData = std::make_unique<CookedAudio>();

        *(sound->transientCpuData) = _cooker->Cook(std::move(rawAudioOpt).value());
        _pipeline->Run(*(sound->transientCpuData));
        _extractor->Extract(*(sound->transientCpuData), sound->cpuData);

        return sound;
    }
}