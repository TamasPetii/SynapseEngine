#include "ModelManager.h"
#include "Engine/Timer/Timer.h"

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
        return models.at(path);

    log << std::format("[Model Thread Started] : {}", path) << "\n";

    std::shared_ptr<Model> model = std::make_shared<Model>(imageManager, materialManager);
    model->SetBufferArrayIndex(GetAvailableIndex());

    models[path] = model;

    futures.emplace(path, std::async(std::launch::async, &Model::Load, models.at(path), path));
    
    return models.at(path);
}

std::shared_ptr<Model> ModelManager::GetModel(const std::string& path)
{
    std::lock_guard<std::mutex> lock(asyncMutex);

    if (models.find(path) == models.end())
        return nullptr;

    return models.at(path);
}

void ModelManager::Update()
{
    std::lock_guard<std::mutex> lock(asyncMutex);
    
    auto completedFutures = AsyncManager::CompleteFinishedFutures();

    for (auto path : completedFutures)
    {
        log << std::format("[Model Thread Finished] : {}", path) << "\n";

        auto model = models.at(path);
        if (model && model->state == LoadState::GpuUploaded)
        {
            static_cast<ModelDevicesAddresses*>(deviceAddresses->GetHandler())[model->GetBufferArrayIndex()] = ModelDevicesAddresses{
                .vertexBufferAddress = model->GetVertexBuffer()->GetAddress(),
                .indexBufferAddress = model->GetIndexBuffer()->GetAddress(),
                .materialBufferAddress = model->GetMaterialBuffer()->GetAddress(),
                .nodeTransformBufferAddress = model->GetNodeTransformBuffer()->GetAddress()
            };
            
            model->state = LoadState::Ready;
        }
    }
}
