#include "StaticMeshBuilder.h"
#include "Engine/Mesh/Source/File/FileMeshSource.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/Serialization/Schema/Models/GpuBatchedModelSchema.h"

namespace Syn
{
    StaticMeshBuilder::StaticMeshBuilder(
        std::unique_ptr<IMeshLoaderRegistry> registry,
        std::unique_ptr<IMeshProcessorPipeline> pipeline,
        std::unique_ptr<IGpuModelConverter> converter,
        std::unique_ptr<IModelCooker> cooker) :
        _registry(std::move(registry)),
        _cooker(std::move(cooker)),
        _pipeline(std::move(pipeline)),
        _converter(std::move(converter))
    {}

    void StaticMeshBuilder::RegisterLoader(std::shared_ptr<IMeshLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void StaticMeshBuilder::RegisterProcessor(std::unique_ptr<IMeshProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
	}

    std::shared_ptr<StaticMesh> StaticMeshBuilder::BuildFromFile(const std::string& filePath)
    {
        std::filesystem::path srcPath(filePath);

        const char* appDataPath = std::getenv("APPDATA");
        std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
        std::filesystem::path saveDir = baseDir / "Synapse" / "Cache" / "Models";

        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directories(saveDir);
        }

        std::filesystem::path cachePath = saveDir / srcPath.filename();
        cachePath.replace_extension(".synmodel");

        auto staticMesh = std::make_shared<StaticMesh>();
        staticMesh->transientCpuData = std::make_unique<CookedModel>();
        staticMesh->transientGpuData = std::make_unique<GpuBatchedModel>();

        auto serializer = ServiceLocator::GetSerializer();

        bool useCache = false;
        if (std::filesystem::exists(cachePath) && std::filesystem::exists(srcPath)) {
            if (std::filesystem::last_write_time(cachePath) >= std::filesystem::last_write_time(srcPath)) {
                useCache = true;
            }
        }

        if (useCache && serializer) {
            if (serializer->LoadFromFile(cachePath, *(staticMesh->transientGpuData))) {
                Info("Loaded {} from binary cache.", srcPath.filename().string());
                return staticMesh;
            }
            Warning("Cache corrupted for {}, rebuilding.", srcPath.filename().string());
        }

        Info("Cooking {} from source...", srcPath.filename().string());

        std::string ext = srcPath.extension().string();
        IMeshLoader* loader = _registry->GetLoaderForExtension(ext);
        if (!loader) return nullptr;

        FileMeshSource source(filePath, loader);
        auto generatedMesh = BuildFromSource(source);

        if (!generatedMesh || !generatedMesh->transientGpuData) 
            return nullptr;

        if (serializer) {
            serializer->SaveToFile(cachePath, *(generatedMesh->transientGpuData));
        }

        return generatedMesh;
    }

    std::shared_ptr<StaticMesh> StaticMeshBuilder::BuildFromSource(IMeshSource& source)
    {
        auto rawModelOpt = source.Produce();

        if (!rawModelOpt) 
            return nullptr;

        auto staticMesh = std::make_shared<StaticMesh>();
        staticMesh->transientCpuData = std::make_unique<CookedModel>();
        staticMesh->transientGpuData = std::make_unique<GpuBatchedModel>();

        *(staticMesh->transientCpuData) = _cooker->Cook(std::move(rawModelOpt).value());
        _pipeline->Run(*(staticMesh->transientCpuData));
        *(staticMesh->transientGpuData) = _converter->Convert(*(staticMesh->transientCpuData));

        return staticMesh;
    }
}