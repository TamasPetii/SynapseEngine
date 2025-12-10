#pragma once
#include <array>
#include <vector>
#include <memory>
#include <cstring>
#include <functional>
#include <cmath>
#include <iostream>

#include "Engine/EngineApi.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Config.h"
#include "BaseManagers/DynamicSizeBuffer.h"
#include "Engine/Registry/Registry.h"

#include "ModelManager.h"
#include "GeometryManager.h"

struct ShadowLightResources
{
    std::shared_ptr<DynamicSizeBuffer> modelIndirectDrawBuffer;
    std::shared_ptr<DynamicSizeBuffer> modelInstanceAddressListBuffer;
    std::vector<std::shared_ptr<DynamicSizeBuffer>> modelInstanceIndexBuffers;

    std::shared_ptr<DynamicSizeBuffer> shapeIndirectDrawBuffer;
    std::shared_ptr<DynamicSizeBuffer> shapeInstanceAddressListBuffer;
    std::vector<std::shared_ptr<DynamicSizeBuffer>> shapeInstanceIndexBuffers;

    ShadowLightResources() {
        modelIndirectDrawBuffer = std::make_shared<DynamicSizeBuffer>();
        modelInstanceAddressListBuffer = std::make_shared<DynamicSizeBuffer>();

        shapeIndirectDrawBuffer = std::make_shared<DynamicSizeBuffer>();
        shapeInstanceAddressListBuffer = std::make_shared<DynamicSizeBuffer>();
    }
};

template <typename ShadowComponent>
class LightShadowBufferManager
{
public:
    LightShadowBufferManager(std::shared_ptr<ModelManager> modelManager, std::shared_ptr<GeometryManager> shapeManager);
    ~LightShadowBufferManager() = default;

    void Update(std::shared_ptr<Registry> registry, uint32_t frameIndex);

    const auto& GetModelInstanceAddressBuffer(uint32_t frameIndex) const { return modelInstanceAddressBuffers[frameIndex]; }
    const auto& GetModelIndirectAddressBuffer(uint32_t frameIndex) const { return modelIndirectAddressBuffers[frameIndex]; }

    const auto& GetShapeInstanceAddressBuffer(uint32_t frameIndex) const { return shapeInstanceAddressBuffers[frameIndex]; }
    const auto& GetShapeIndirectAddressBuffer(uint32_t frameIndex) const { return shapeIndirectAddressBuffers[frameIndex]; }

    const auto& GetShadowResources(uint32_t frameIndex, uint32_t shadowIndex) { return shadowResources[frameIndex][shadowIndex]; }

private:
    void InitResources();

    bool EnsureBuffer(std::shared_ptr<DynamicSizeBuffer>& buffer, uint32_t elementCount, uint32_t elementSize,
        uint32_t baseElementCount, VkBufferUsageFlags usage);

    bool UpdateShadowType(
        uint32_t frameIndex,
        uint32_t shadowCount,
        uint32_t itemCount,
        std::shared_ptr<DynamicSizeBuffer> masterIndirectBuffer,
        std::shared_ptr<DynamicSizeBuffer> ShadowLightResources::* indirectBufferMember,
        std::shared_ptr<DynamicSizeBuffer> ShadowLightResources::* addressListBufferMember,
        std::vector<std::shared_ptr<DynamicSizeBuffer>> ShadowLightResources::* instanceBuffersMember,
        std::vector<VkDeviceAddress>& outIndirectAddresses,
        std::vector<VkDeviceAddress>& outInstanceListAddresses
    );

    void UploadTopLevelBuffer(std::shared_ptr<DynamicSizeBuffer>& buffer, const std::vector<VkDeviceAddress>& data);

private:
    std::shared_ptr<ModelManager> modelManager;
    std::shared_ptr<GeometryManager> shapeManager;

    std::array<std::vector<ShadowLightResources>, GlobalConfig::FrameConfig::maxFramesInFlights> shadowResources;

    std::array<uint32_t, GlobalConfig::FrameConfig::maxFramesInFlights> lastModelMasterVersions = { 0 };
    std::array<uint32_t, GlobalConfig::FrameConfig::maxFramesInFlights> lastShapeMasterVersions = { 0 };
    std::array<uint32_t, GlobalConfig::FrameConfig::maxFramesInFlights> lastShadowCounts = { 0 };

    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> modelInstanceAddressBuffers;
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> modelIndirectAddressBuffers;
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> shapeInstanceAddressBuffers;
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> shapeIndirectAddressBuffers;
};

template<typename S>
LightShadowBufferManager<S>::LightShadowBufferManager(std::shared_ptr<ModelManager> modelManager, std::shared_ptr<GeometryManager> shapeManager)
    : modelManager(modelManager), shapeManager(shapeManager)
{
    InitResources();
}

template<typename S>
void LightShadowBufferManager<S>::InitResources()
{
    std::cout << "[InitResources] Allocating per-frame top-level buffers...\n";

    for (uint32_t i = 0; i < GlobalConfig::FrameConfig::maxFramesInFlights; ++i)
    {
        modelInstanceAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        modelIndirectAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        shapeInstanceAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();
        shapeIndirectAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();
    }
}

/// DEBUG VERSION —
template<typename S>
bool LightShadowBufferManager<S>::EnsureBuffer(std::shared_ptr<DynamicSizeBuffer>& buffer, uint32_t elementCount,
    uint32_t elementSize, uint32_t baseElementCount, VkBufferUsageFlags usage)
{
    uint32_t alignedElementCount = static_cast<uint32_t>(std::ceil((elementCount + 1) / (float)baseElementCount)) * baseElementCount;
    uint32_t requiredSizeBytes = alignedElementCount * elementSize;

    bool needRealloc = (buffer->buffer == nullptr || buffer->size != requiredSizeBytes);

    if (needRealloc)
    {
        std::cout << "[EnsureBuffer] REALLOC — elementCount=" << elementCount
            << " elementSize=" << elementSize
            << " alignedCount=" << alignedElementCount
            << " sizeBytes=" << requiredSizeBytes
            << " ptr=" << buffer.get() << "\n";

        Vk::BufferConfig config;
        config.size = requiredSizeBytes;
        config.usage = usage;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        buffer->buffer = std::make_shared<Vk::Buffer>(config);
        buffer->buffer->MapMemory();
        buffer->size = requiredSizeBytes;
        buffer->version++;

        std::cout << "   -> New GPU Address: " << buffer->buffer->GetAddress() << "\n";
        return true;
    }

    return false;
}

template<typename S>
bool LightShadowBufferManager<S>::UpdateShadowType(
    uint32_t frameIndex,
    uint32_t shadowCount,
    uint32_t itemCount,
    std::shared_ptr<DynamicSizeBuffer> masterIndirectBuffer,
    std::shared_ptr<DynamicSizeBuffer> ShadowLightResources::* indirectBufferMember,
    std::shared_ptr<DynamicSizeBuffer> ShadowLightResources::* addressListBufferMember,
    std::vector<std::shared_ptr<DynamicSizeBuffer>> ShadowLightResources::* instanceBuffersMember,
    std::vector<VkDeviceAddress>& outIndirectAddresses,
    std::vector<VkDeviceAddress>& outInstanceListAddresses)
{
    if (itemCount == 0 || masterIndirectBuffer == nullptr || masterIndirectBuffer->buffer == nullptr) {
        std::cout << "  -> SKIP (no items or master buffer missing)\n";
        return false;
    }

    bool anyAddressChanged = false;
    auto* masterCommands = static_cast<VkDrawIndirectCommand*>(masterIndirectBuffer->buffer->GetHandler());

    const uint32_t indirectBaseSize = 16;
    const uint32_t instanceBaseSize = GlobalConfig::BufferConfig::instanceBufferBaseSize;

    outIndirectAddresses.reserve(shadowCount);
    outInstanceListAddresses.reserve(shadowCount);

    for (uint32_t i = 0; i < shadowCount; ++i)
    {
        auto& res = shadowResources[frameIndex][i];

        auto& indirectBuffer = res.*indirectBufferMember;

        bool indirectResized = EnsureBuffer(indirectBuffer, itemCount, sizeof(VkDrawIndirectCommand),
            indirectBaseSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        VkDeviceAddress indirAddr = indirectBuffer->buffer->GetAddress();
        outIndirectAddresses.push_back(indirAddr);
        if (indirectResized) anyAddressChanged = true;

        auto* destCommands = static_cast<VkDrawIndirectCommand*>(indirectBuffer->buffer->GetHandler());
        memcpy(destCommands, masterCommands, itemCount * sizeof(VkDrawIndirectCommand));
        for (uint32_t k = 0; k < itemCount; ++k)
            destCommands[k].instanceCount = 0;

        auto& instanceBuffers = res.*instanceBuffersMember;

        if (instanceBuffers.size() < itemCount) {
            instanceBuffers.resize(itemCount);
            for (auto& ptr : instanceBuffers)
                if (!ptr) ptr = std::make_shared<DynamicSizeBuffer>();
        }

        std::vector<VkDeviceAddress> currentShadowItemAddresses;
        currentShadowItemAddresses.reserve(itemCount);

        bool listContentChanged = false;

        for (uint32_t k = 0; k < itemCount; ++k)
        {
            bool instResized = EnsureBuffer(instanceBuffers[k], instanceBaseSize, sizeof(uint32_t),
                instanceBaseSize,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

            VkDeviceAddress addr = instanceBuffers[k]->buffer->GetAddress();
            currentShadowItemAddresses.push_back(addr);

            if (instResized) listContentChanged = true;
        }

        auto& addressListBuffer = res.*addressListBufferMember;

        bool listResized = EnsureBuffer(addressListBuffer, itemCount, sizeof(VkDeviceAddress),
            indirectBaseSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        if (listResized || listContentChanged) {
            memcpy(addressListBuffer->buffer->GetHandler(),
                currentShadowItemAddresses.data(),
                itemCount * sizeof(VkDeviceAddress));
            std::cout << "     -> AddressListBuffer updated (resized=" << listResized
                << ", contentChanged=" << listContentChanged << ")\n";
        }

        VkDeviceAddress listAddr = addressListBuffer->buffer->GetAddress();
        outInstanceListAddresses.push_back(listAddr);

        if (listResized) anyAddressChanged = true;
    }

    return anyAddressChanged;
}

template<typename S>
void LightShadowBufferManager<S>::UploadTopLevelBuffer(std::shared_ptr<DynamicSizeBuffer>& buffer,
    const std::vector<VkDeviceAddress>& data)
{
    std::cout << "[UploadTopLevelBuffer] count=" << data.size()
        << " bufferPtr=" << buffer.get() << "\n";

    if (data.empty()) return;

    bool resized = EnsureBuffer(buffer, data.size(), sizeof(VkDeviceAddress), 16,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    std::cout << "   -> Uploading. resized=" << resized
        << " gpuAddr=" << buffer->buffer->GetAddress() << "\n";

    memcpy(buffer->buffer->GetHandler(), data.data(),
        data.size() * sizeof(VkDeviceAddress));
}

template<typename S>
void LightShadowBufferManager<S>::Update(std::shared_ptr<Registry> registry, uint32_t frameIndex)
{
    uint32_t modelCount = modelManager->GetCurrentCount();
    uint32_t shapeCount = shapeManager->GetCurrentCount();

    auto shadowPool = registry->GetPool<S>();
    uint32_t shadowCount = shadowPool ? shadowPool->GetDenseSize() : 0;

    if (shadowCount == 0) {
        std::cout << " -> No shadow casters.\n";
        return;
    }

    auto& currentFrameResources = shadowResources[frameIndex];
    if (currentFrameResources.size() < shadowCount) {
        std::cout << "Resizing shadowResources to shadowCount=" << shadowCount << "\n";
        currentFrameResources.resize(shadowCount);
    }

    bool masterModelChanged = (modelManager->GetIndirectDrawBuffer(frameIndex)->version != lastModelMasterVersions[frameIndex]);
    bool masterShapeChanged = (shapeManager->GetIndirectDrawBuffer(frameIndex)->version != lastShapeMasterVersions[frameIndex]);
    bool shadowCountChanged = (shadowCount != lastShadowCounts[frameIndex]);

    lastModelMasterVersions[frameIndex] = modelManager->GetIndirectDrawBuffer(frameIndex)->version;
    lastShapeMasterVersions[frameIndex] = shapeManager->GetIndirectDrawBuffer(frameIndex)->version;
    lastShadowCounts[frameIndex] = shadowCount;

    std::vector<VkDeviceAddress> gpuModelIndirectAddresses;
    std::vector<VkDeviceAddress> gpuModelAddressLists;

    bool modelsInternalChanged = UpdateShadowType(
        frameIndex, shadowCount, modelCount,
        modelManager->GetIndirectDrawBuffer(frameIndex),
        &ShadowLightResources::modelIndirectDrawBuffer,
        &ShadowLightResources::modelInstanceAddressListBuffer,
        &ShadowLightResources::modelInstanceIndexBuffers,
        gpuModelIndirectAddresses,
        gpuModelAddressLists
    );

    if (masterModelChanged || shadowCountChanged || modelsInternalChanged ||
        modelIndirectAddressBuffers[frameIndex]->buffer == nullptr)
    {
        std::cout << "[MODEL] Uploading top-level buffers...\n";
        UploadTopLevelBuffer(modelIndirectAddressBuffers[frameIndex], gpuModelIndirectAddresses);
        UploadTopLevelBuffer(modelInstanceAddressBuffers[frameIndex], gpuModelAddressLists);
    }

    std::vector<VkDeviceAddress> gpuShapeIndirectAddresses;
    std::vector<VkDeviceAddress> gpuShapeAddressLists;

    bool shapesInternalChanged = UpdateShadowType(
        frameIndex, shadowCount, shapeCount,
        shapeManager->GetIndirectDrawBuffer(frameIndex),
        &ShadowLightResources::shapeIndirectDrawBuffer,
        &ShadowLightResources::shapeInstanceAddressListBuffer,
        &ShadowLightResources::shapeInstanceIndexBuffers,
        gpuShapeIndirectAddresses,
        gpuShapeAddressLists
    );

    if (masterShapeChanged || shadowCountChanged || shapesInternalChanged ||
        shapeIndirectAddressBuffers[frameIndex]->buffer == nullptr)
    {
        std::cout << "[SHAPE] Uploading top-level buffers...\n";
        UploadTopLevelBuffer(shapeIndirectAddressBuffers[frameIndex], gpuShapeIndirectAddresses);
        UploadTopLevelBuffer(shapeInstanceAddressBuffers[frameIndex], gpuShapeAddressLists);
    }
}