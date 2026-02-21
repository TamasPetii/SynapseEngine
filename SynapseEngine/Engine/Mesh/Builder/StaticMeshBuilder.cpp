#include "StaticMeshBuilder.h"
#include "Engine/Mesh/Source/File/FileMeshSource.h"
#include "Engine/Mesh/Data/Cooked/CookedUtils.h"

namespace Syn
{
    StaticMeshBuilder::StaticMeshBuilder(std::unique_ptr<IMeshLoaderRegistry> registry, std::unique_ptr<IMeshProcessorPipeline> pipeline) :
		_registry(std::move(registry)), 
        _pipeline(std::move(pipeline))
    {
    }

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
        std::string ext = std::filesystem::path(filePath).extension().string();
        IMeshLoader* loader = _registry->GetLoaderForExtension(ext); // Sima tagváltozóból olvassuk!

        if (!loader) 
            return nullptr;

        FileMeshSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::shared_ptr<StaticMesh> StaticMeshBuilder::BuildFromSource(IMeshSource& source)
    {
        auto rawModelOpt = source.Produce();

        if (!rawModelOpt) 
            return nullptr;

        auto staticMesh = std::make_shared<StaticMesh>();
        staticMesh->cpuData = CookedUtils::ConvertRawModel(std::move(rawModelOpt).value());

        _pipeline->Run(staticMesh->cpuData);

        // staticMesh->gpuData = UploadToGpu(staticMesh->cpuData);

        return staticMesh;
    }
}