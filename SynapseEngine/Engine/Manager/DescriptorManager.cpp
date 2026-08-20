#include "DescriptorManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Vk/Descriptor/DescriptorLayoutBuilder.h"

namespace Syn {
    DescriptorManager::DescriptorManager(uint32_t framesInFlight) : _framesInFlight(framesInFlight) {
        InitializeBindlessSetup();
    }

    DescriptorManager::~DescriptorManager() {
        auto device = ServiceLocator::Get<Vk::Context>()->GetDevice()->Handle();
        if (_bindlessLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, _bindlessLayout, nullptr);
            _bindlessLayout = VK_NULL_HANDLE;
        }
    }

    void DescriptorManager::InitializeBindlessSetup() {
        Vk::DescriptorLayoutBuilder layoutBuilder;
        layoutBuilder.AddBindlessBinding(BINDING_SAMPLERS, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL, MAX_SAMPLERS);
        layoutBuilder.AddBindlessBinding(BINDING_TEXTURES, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, MAX_IMAGES);
        layoutBuilder.AddBindlessBinding(BINDING_VIDEO_TEXTURES, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, MAX_VIDEOS);
        layoutBuilder.AddBindlessBinding(BINDING_CUBE_TEXTURES, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, MAX_ENVIRONMENTS);

        _bindlessLayout = layoutBuilder.Build(Vk::DescriptorLayoutType::DescriptorBuffer);
        _bindlessBuffer = std::make_unique<Vk::DescriptorBuffer>(_bindlessLayout);
    }

    void DescriptorManager::Update() {
        std::lock_guard<std::mutex> lock(_staleMutex);

        for (auto it = _staleGpuBuffers.begin(); it != _staleGpuBuffers.end();) {
            if (it->framesToLive > 0) {
                it->framesToLive--;
                ++it;
            }
            else {
                it = _staleGpuBuffers.erase(it);
            }
        }

        for (auto it = _staleMappedBuffers.begin(); it != _staleMappedBuffers.end();) {
            if (it->framesToLive > 0) {
                it->framesToLive--;
                ++it;
            }
            else {
                it = _staleMappedBuffers.erase(it);
            }
        }
    }

    void DescriptorManager::RecordSync(VkCommandBuffer cmd) {
        if (auto staleBuffers = _bindlessBuffer->RecordSync(cmd); staleBuffers.mapped || staleBuffers.gpu) {
            std::lock_guard<std::mutex> lock(_staleMutex);
            _staleMappedBuffers.push_back({ staleBuffers.mapped, _framesInFlight });
            _staleGpuBuffers.push_back({ staleBuffers.gpu, _framesInFlight });
        }
    }

    void DescriptorManager::WriteSampler(uint32_t index, VkSampler sampler) {
        _bindlessBuffer->WriteSampler(BINDING_SAMPLERS, index, sampler);
    }

    void DescriptorManager::WriteTexture(uint32_t index, VkImageView view) {
        _bindlessBuffer->WriteSampledImage(BINDING_TEXTURES, index, view);
    }

    void DescriptorManager::FillTextures(VkImageView defaultView) {
        _bindlessBuffer->FillSampledImages(BINDING_TEXTURES, MAX_IMAGES, defaultView);
    }

    void DescriptorManager::WriteVideoTexture(uint32_t index, VkImageView view) {
        _bindlessBuffer->WriteSampledImage(BINDING_VIDEO_TEXTURES, index, view);
    }

    void DescriptorManager::WriteCubeTexture(uint32_t index, VkImageView view) {
        _bindlessBuffer->WriteSampledImage(BINDING_CUBE_TEXTURES, index, view);
    }
}