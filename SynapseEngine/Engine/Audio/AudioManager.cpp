#include "AudioManager.h"

namespace Syn
{
    AudioManager::AudioManager(std::shared_ptr<AudioBuilder> builder)
        : _builder(std::move(builder))
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

        SetResourceState(entryId, ResourceState::Ready);
        MarkDirty(entryId);
    }

    void AudioManager::FinalizeResource(EntryType& entry)
    {
        if (entry.resource) {
            entry.resource->transientCpuData.reset();
        }
    }
}