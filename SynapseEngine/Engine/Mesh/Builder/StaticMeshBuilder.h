#pragma once
#include "Engine/SynApi.h"

#include "../Loader/IMeshLoader.h"
#include "../Loader/IMeshLoaderRegistry.h"
#include "../Processor/IMeshProcessor.h"
#include "../Processor/IMeshProcessorPipeline.h"
#include "../Source/IMeshSource.h"
#include "../Data/StaticMesh.h"

#include <memory>

namespace Syn
{
    class SYN_API StaticMeshBuilder
    {
    public:
        StaticMeshBuilder(std::unique_ptr<IMeshLoaderRegistry> registry, std::unique_ptr<IMeshProcessorPipeline> pipeline);

        StaticMeshBuilder(const StaticMeshBuilder&) = delete;
        StaticMeshBuilder& operator=(const StaticMeshBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IMeshLoader> loader, int priority = 0);
		void RegisterProcessor(std::unique_ptr<IMeshProcessor> processor);
        std::shared_ptr<StaticMesh> BuildFromFile(const std::string& filePath);
        std::shared_ptr<StaticMesh> BuildFromSource(IMeshSource& source);
    private:
        std::unique_ptr<IMeshLoaderRegistry> _registry;
        std::unique_ptr<IMeshProcessorPipeline> _pipeline;
    };
}