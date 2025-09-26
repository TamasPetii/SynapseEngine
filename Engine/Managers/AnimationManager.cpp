#include "AnimationManager.h"
#include <iostream>

std::shared_ptr<Animation> AnimationManager::LoadAnimation(const std::string& path)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    if (animations.find(path) != animations.end())
        return animations.at(path)->object;

    log << std::format("[Animation Thread Started] : {}", path) << "\n";

    std::shared_ptr<Animation> animation = std::make_shared<Animation>();
    animation->SetBufferArrayIndex(GetAvailableIndex());

    animations.insert(std::make_pair(path, std::make_shared<VersionedObject<Animation>>(animation)));

    futures.emplace(path, std::async(std::launch::async, &Animation::Load, animations.at(path)->object, path));

    return animations.at(path)->object;
}

std::shared_ptr<Animation> AnimationManager::GetAnimation(const std::string& path)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    if (animations.find(path) == animations.end())
        return nullptr;

    if (animations.at(path) == nullptr)
        return nullptr;

    return animations.at(path)->object;
}

AnimationManager::AnimationManager()
{
}

AnimationManager::~AnimationManager()
{
    animations.clear();
}

void AnimationManager::Update(uint32_t frameIndex)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    DeviceAddressedManager<VkDeviceAddress>::Update(frameIndex, ArrayIndexedManager::GetCurrentCount(), GlobalConfig::BufferConfig::animationBufferBaseSize);

    auto completedFutures = AsyncManager::CompleteFinishedFutures();

    for (auto path : completedFutures)
    {
        log << std::format("[Animation Thread Finished] : {}", path) << "\n";

        auto animation = animations.at(path)->object;

        if (animation && animation->state == LoadState::GpuUploaded)
            animation->state = LoadState::Ready; 

        log << std::format("Animation Vertex Bone Buffer Uploaded: {} ", animation->GetBufferArrayIndex()) << "\n";
    }

    for (auto& [path, versionedObject] : animations)
    {
        if (versionedObject == nullptr)
            continue;

        auto animation = versionedObject->object;

        //Model state ready??? Should tell if its uploaded to gpu or not??? But for multiple frames idk...
        if (animation && animation->state == LoadState::Ready && deviceAddressBuffers[frameIndex]->version != versionedObject->versions[frameIndex])
        {
            versionedObject->versions[frameIndex] = deviceAddressBuffers[frameIndex]->version;

            auto bufferHandler = static_cast<VkDeviceAddress*>(deviceAddressBuffers[frameIndex]->buffer->GetHandler());
            bufferHandler[animation->GetBufferArrayIndex()] = animation->GetVertexBoneBuffer()->GetAddress();

            std::cout << std::format("Animation {} updated in frame {} with version {}", path, frameIndex, versionedObject->versions[frameIndex]) << std::endl;
        }
    }
}
