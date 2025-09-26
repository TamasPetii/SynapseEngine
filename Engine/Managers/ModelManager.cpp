#include "ModelManager.h"
#include "Engine/Timer/Timer.h"
#include <iostream>
#include <format>

ModelManager::ModelManager(std::shared_ptr<ImageManager> imageManager, std::shared_ptr<MaterialManager> materialManager) :
    imageManager(imageManager),
    materialManager(materialManager)
{
}

ModelManager::~ModelManager()
{
    models.clear();
}

std::shared_ptr<Model> ModelManager::LoadModel(const std::string& path)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    if (models.find(path) != models.end())
        return models.at(path)->object;

    log << std::format("[Model Thread Started] : {}", path) << "\n";

    std::shared_ptr<Model> model = std::make_shared<Model>(imageManager, materialManager);
    model->SetBufferArrayIndex(GetAvailableIndex());

    models[path] = std::make_shared<VersionedObject<Model>>(model);

    futures.emplace(path, std::async(std::launch::async, &Model::Load, models.at(path)->object, path));
    
    return models.at(path)->object;
}

std::shared_ptr<Model> ModelManager::GetModel(const std::string& path)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    if (models.find(path) == models.end())
        return nullptr;

    if (models.at(path) == nullptr)
        return nullptr;

    return models.at(path)->object;
}

void ModelManager::Update(uint32_t frameIndex)
{
    std::lock_guard<std::mutex> lock(asyncMutex);
    
    //Todo: Mechanism to handle model delete, shrink buffer and reassign model's buffer array index!
    DeviceAddressedManager<ModelDeviceAddresses>::Update(frameIndex, ArrayIndexedManager::GetCurrentCount(), GlobalConfig::BufferConfig::modelAddressBufferBaseSize);

    auto completedFutures = AsyncManager::CompleteFinishedFutures();

    for (auto path : completedFutures)
    {
        log << std::format("[Model Thread Finished] : {}", path) << "\n";

        auto model = models.at(path)->object;

        if (model && model->state == LoadState::GpuUploaded)
            model->state = LoadState::Ready;
    }

    for (auto& [path, versionedObject] : models)
    {
        auto model = versionedObject->object;

        //Model state ready??? Should tell if its uploaded to gpu or not??? But for multiple frames idk...
        if (model && model->state == LoadState::Ready && deviceAddressBuffers[frameIndex]->version != versionedObject->versions[frameIndex])
        {
            versionedObject->versions[frameIndex] = deviceAddressBuffers[frameIndex]->version;

            auto bufferHandler = static_cast<ModelDeviceAddresses*>(deviceAddressBuffers[frameIndex]->buffer->GetHandler());
            bufferHandler[model->GetBufferArrayIndex()] = ModelDeviceAddresses{
                .vertexBufferAddress = model->GetVertexBuffer()->GetAddress(),
                .indexBufferAddress = model->GetIndexBuffer()->GetAddress(),
                .materialBufferAddress = model->GetMaterialBuffer()->GetAddress(),
                .nodeTransformBufferAddress = model->GetNodeTransformBuffer()->GetAddress()
            };

            std::cout << std::format("Model {} updated in frame {} with version {}", path, frameIndex, versionedObject->versions[frameIndex]) << std::endl;
        }
    }

    /*
    //Todo: GPU LOD?
    for (uint32_t frameIndex = 0; frameIndex < indirectCommandBuffers.size(); ++frameIndex)
    {
        static_cast<VkDrawIndirectCommand*>(indirectCommandBuffers[frameIndex]->GetHandler())[model->GetBufferArrayIndex()] = VkDrawIndirectCommand{
            .vertexCount = model->GetIndexCount(),
            .instanceCount = 0,
            .firstVertex = 0,
            .firstInstance = 0
        };
        .instanceIndexBufferAddress = model->GetInstanceIndexBuffer()->GetAddress()
    }
    */
}
