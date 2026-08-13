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
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Audio/Builder/AudioBuilder.h"
#include "Engine/Audio/Data/Sound.h"
#include <memory>
#include <string>
#include <functional>

namespace Syn
{
    using AudioSourceFactory = std::function<std::unique_ptr<IAudioSource>()>;
    using PreviewAllocateCallback = std::function<void(uint32_t resourceId)>;
    using PreviewMarkDirtyCallback = std::function<void(uint32_t resourceId)>;

    class SYN_API AudioManager : public BaseResourceManager<Sound>
    {
    public:
        AudioManager(std::shared_ptr<AudioBuilder> builder, 
            PreviewAllocateCallback previewAllocateCallback = nullptr,
            PreviewMarkDirtyCallback previewMarkDirtyCallback = nullptr);
        ~AudioManager() override = default;

        uint32_t LoadAudioAsync(const std::string& filePath);
        uint32_t LoadAudioFromSourceAsync(const std::string& name, AudioSourceFactory factory);

        uint32_t LoadAudioSync(const std::string& filePath);
        uint32_t LoadAudioFromSourceSync(const std::string& name, AudioSourceFactory factory);

    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        std::shared_ptr<AudioBuilder> _builder;
        PreviewAllocateCallback _previewAllocateCallback;
        PreviewMarkDirtyCallback _previewMarkDirtyCallback;
    };
}