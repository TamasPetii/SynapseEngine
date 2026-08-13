// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "StaticMeshBuilder.h"
#include "Engine/Mesh/Source/File/FileMeshSource.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Serialization/Serializer.h"
#include "Engine/Serialization/Schema/Models/StaticMeshSchema.h"

namespace Syn
{
    StaticMeshBuilder::StaticMeshBuilder(
        std::unique_ptr<IMeshLoaderRegistry> registry,
        std::unique_ptr<IMeshProcessorPipeline> pipeline,
        std::unique_ptr<IGpuModelConverter> converter,
        std::unique_ptr<IModelCooker> cooker,
        std::unique_ptr<ICpuModelExtractor> extractor,
        std::unique_ptr<ICpuModelProcessorPipeline> cpuPipeline) :
        _registry(std::move(registry)),
        _cooker(std::move(cooker)),
        _meshPipeline(std::move(pipeline)),
        _converter(std::move(converter)),
        _extractor(std::move(extractor)),
        _cpuModelPipeline(std::move(cpuPipeline))
    {}

    void StaticMeshBuilder::RegisterLoader(std::shared_ptr<IMeshLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void StaticMeshBuilder::RegisterCpuModelProcessor(std::unique_ptr<ICpuModelProcessor> processor)
    {
        _cpuModelPipeline->AddProcessor(std::move(processor));
    }

    void StaticMeshBuilder::RegisterMeshProcessor(std::unique_ptr<IMeshProcessor> processor)
    {
        _meshPipeline->AddProcessor(std::move(processor));
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

        auto serializer = ServiceLocator::Get<Serializer>();

        bool useCache = false;
        if (std::filesystem::exists(cachePath) && std::filesystem::exists(srcPath)) {
            if (std::filesystem::last_write_time(cachePath) >= std::filesystem::last_write_time(srcPath)) {
                useCache = true;
            }
        }

        if (useCache && serializer) {
            auto staticMesh = std::make_shared<StaticMesh>();
            staticMesh->transientCpuData = std::make_unique<CookedModel>();
            staticMesh->transientGpuData = std::make_unique<GpuBatchedModel>();

            if (serializer->LoadFromFile(cachePath, *staticMesh)) {
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
            serializer->SaveToFile(cachePath, *generatedMesh);
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
        _meshPipeline->Run(*(staticMesh->transientCpuData));
        *(staticMesh->transientGpuData) = _converter->Convert(*(staticMesh->transientCpuData));

        _extractor->Extract(*(staticMesh->transientGpuData), staticMesh->cpuData);
        _cpuModelPipeline->Run(staticMesh->cpuData);

        return staticMesh;
    }
}