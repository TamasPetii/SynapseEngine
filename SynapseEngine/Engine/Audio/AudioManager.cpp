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

#include "AudioManager.h"

namespace Syn
{
    AudioManager::AudioManager(std::shared_ptr<AudioBuilder> builder,
        PreviewAllocateCallback previewAllocateCallback,
        PreviewMarkDirtyCallback previewMarkDirtyCallback)
        : 
        _builder(std::move(builder)),
        _previewAllocateCallback(std::move(previewAllocateCallback)),
        _previewMarkDirtyCallback(std::move(previewMarkDirtyCallback))
    {}

    uint32_t AudioManager::LoadAudioAsync(const std::string& filePath)
    {
        return InternalLoadAsync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t AudioManager::LoadAudioFromSourceAsync(const std::string& name, AudioSourceFactory factory)
    {
        return InternalLoadAsync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<Sound>(nullptr);
            });
    }

    uint32_t AudioManager::LoadAudioSync(const std::string& filePath)
    {
        return InternalLoadSync(filePath, [this, filePath]() {
            return _builder->BuildFromFile(filePath);
            });
    }

    uint32_t AudioManager::LoadAudioFromSourceSync(const std::string& name, AudioSourceFactory factory)
    {
        return InternalLoadSync(name, [this, factory]() {
            if (auto source = factory()) {
                return _builder->BuildFromSource(*source);
            }
            return std::shared_ptr<Sound>(nullptr);
            });
    }

    void AudioManager::StartGpuUpload(EntryType& entry)
    {
        uint32_t entryId = _pathToId.at(entry.path);

        std::lock_guard lock(_mutex);
        FinalizeResource(entry);

        if (_previewMarkDirtyCallback) {
            _previewMarkDirtyCallback(entryId);
        }

        SetResourceState(entryId, ResourceState::Ready);
        MarkDirty(entryId);
    }

    void AudioManager::FinalizeResource(EntryType& entry)
    {
        if (entry.resource) 
        {
            uint32_t entryId = _pathToId.at(entry.path);

            if (_previewAllocateCallback) {
                _previewAllocateCallback(entryId);
            }

            entry.resource->transientCpuData.reset();
        }
    }
}