#pragma once
#include "Engine/SynApi.h"

#include "../Loader/IMeshLoader.h"
#include "../Loader/IMeshLoaderRegistry.h"
#include "../Processor/MeshProcessor/IMeshProcessor.h"
#include "../Processor/MeshProcessor/IMeshProcessorPipeline.h"
#include "../Source/IMeshSource.h"
#include "../Data/StaticMesh.h"

#include "../Converter/IModelCooker.h"
#include "../Converter/IGpuModelConverter.h"
#include "../Uploader/IGpuModelUploader.h"

#include "../Converter/ICpuModelExtractor.h"
#include "../Processor/CpuModelProcessor/ICpuModelProcessorPipeline.h"

#include <memory>

namespace Syn
{
    class SYN_API StaticMeshBuilder
    {
    public:
        StaticMeshBuilder(
            std::unique_ptr<IMeshLoaderRegistry> registry,
            std::unique_ptr<IMeshProcessorPipeline> pipeline,
            std::unique_ptr<IGpuModelConverter> converter,
            std::unique_ptr<IModelCooker> cooker,
            std::unique_ptr<ICpuModelExtractor> extractor,
            std::unique_ptr<ICpuModelProcessorPipeline> cpuPipeline
        );

        StaticMeshBuilder(const StaticMeshBuilder&) = delete;
        StaticMeshBuilder& operator=(const StaticMeshBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IMeshLoader> loader, int priority = 0);
        void RegisterMeshProcessor(std::unique_ptr<IMeshProcessor> processor);
        void RegisterCpuModelProcessor(std::unique_ptr<ICpuModelProcessor> processor);

        std::shared_ptr<StaticMesh> BuildFromFile(const std::string& filePath);
        std::shared_ptr<StaticMesh> BuildFromSource(IMeshSource& source);
    private:
        std::unique_ptr<IMeshLoaderRegistry> _registry;
        std::unique_ptr<IMeshProcessorPipeline> _meshPipeline;
        std::unique_ptr<IGpuModelConverter> _converter;
        std::unique_ptr<IModelCooker> _cooker;
        std::unique_ptr<ICpuModelExtractor> _extractor;
        std::unique_ptr<ICpuModelProcessorPipeline> _cpuModelPipeline;
    };
}