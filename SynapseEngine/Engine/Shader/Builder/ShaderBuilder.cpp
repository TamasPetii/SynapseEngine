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

#include "ShaderBuilder.h"
#include "Engine/Shader/Source/File/FileShaderSource.h"
#include "Engine/Logger/SynLog.h"
#include <filesystem>
#include "Engine/Utils/PathUtils.h"
#include "Engine/EnginePaths.h"

namespace Syn 
{
    ShaderBuilder::ShaderBuilder(
        std::unique_ptr<IShaderLoaderRegistry> loaderRegistry,
        std::unique_ptr<IShaderProcessorPipeline> processorPipeline,
        std::unique_ptr<IShaderCooker> cooker,
        std::unique_ptr<IGpuShaderConverter> converter,
        std::unique_ptr<ICpuShaderExtractor> extractor,
        std::unique_ptr<IShaderDependencyResolver> dependencyResolver) :
        _loaderRegistry(std::move(loaderRegistry)),
        _processorPipeline(std::move(processorPipeline)),
        _cooker(std::move(cooker)),
        _converter(std::move(converter)),
        _extractor(std::move(extractor)),
        _dependencyResolver(std::move(dependencyResolver))
    {}

    void ShaderBuilder::RegisterLoader(std::shared_ptr<IShaderLoader> loader, int priority) {
        _loaderRegistry->Register(loader, priority);
    }

    void ShaderBuilder::RegisterProcessor(std::unique_ptr<IShaderProcessor> processor) {
        _processorPipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Shader> ShaderBuilder::BuildFromFile(const std::string& filePath, VkShaderStageFlagBits stage, const std::vector<std::string>& defines) 
    {
        std::filesystem::path sourcePath(filePath);
        sourcePath = Syn::PathUtils::GetAbsolutePath(sourcePath);

        auto cacheDir = EnginePaths::GetShadersCacheDir();

        if (!std::filesystem::exists(cacheDir)) {
            std::filesystem::create_directories(cacheDir);
        }

        std::string cacheFilename = filePath;
        std::replace(cacheFilename.begin(), cacheFilename.end(), '/', '_');
        std::replace(cacheFilename.begin(), cacheFilename.end(), '\\', '_');
        std::replace(cacheFilename.begin(), cacheFilename.end(), ':', '_');

        for (const auto& def : defines) {
            cacheFilename += "_" + def;
        }

#if defined(SYN_DEBUG)
        cacheFilename += "_debug";
#elif defined(SYN_RELEASE)
        cacheFilename += "_release";
#elif defined(SYN_DIST)
        cacheFilename += "_dist";
#else
        cacheFilename += "_unknown";
#endif

        std::filesystem::path cachePath = cacheDir / (cacheFilename + ".spv");
        bool needsCompile = true;

        if (std::filesystem::exists(cachePath) && std::filesystem::exists(sourcePath)) {
            auto cacheTime = std::filesystem::last_write_time(cachePath);
            needsCompile = false;

            std::unordered_set<std::string> dependencies = _dependencyResolver->ResolveDependencies(sourcePath);

            for (const auto& dep : dependencies) {
                if (std::filesystem::exists(dep)) {
                    auto depTime = std::filesystem::last_write_time(dep);
                    if (depTime > cacheTime) {
                        needsCompile = true;
                        break;
                    }
                }
            }
        }

        if (!needsCompile) {
            std::ifstream file(cachePath, std::ios::ate | std::ios::binary);
            if (file.is_open()) {
                size_t fileSize = (size_t)file.tellg();
                std::vector<uint32_t> spirv(fileSize / sizeof(uint32_t));

                file.seekg(0);
                file.read(reinterpret_cast<char*>(spirv.data()), fileSize);
                file.close();

                Info("Loaded cached shader: {}", filePath);

                auto shader = std::make_shared<Shader>();
                shader->transientGpuData = std::make_unique<GpuShaderData>();
                shader->transientGpuData->spirv = std::move(spirv);
                shader->transientGpuData->stage = stage;

                shader->cpuData.identifier = filePath;
                shader->cpuData.stage = stage;

                _extractor->Extract(*(shader->transientGpuData), shader->cpuData);

                return shader;
            }
        }

        std::string ext = sourcePath.extension().string();
        IShaderLoader* loader = _loaderRegistry->GetLoaderForExtension(ext);

        if (!loader) {
            Error("No loader found for shader extension: {}", ext);
            return nullptr;
        }

        FileShaderSource source(sourcePath.string(), loader, stage, defines);
        auto shader = BuildFromSource(source);

        if (shader && shader->transientGpuData && !shader->transientGpuData->spirv.empty()) {
            std::ofstream outFile(cachePath, std::ios::out | std::ios::binary);
            if (outFile.is_open()) {
                outFile.write(reinterpret_cast<const char*>(shader->transientGpuData->spirv.data()),
                    shader->transientGpuData->spirv.size() * sizeof(uint32_t));
                outFile.close();
                Info("Compiled and cached shader: {}", sourcePath.string());
            }
        }

        return shader;
    }

    std::shared_ptr<Shader> ShaderBuilder::BuildFromSource(IShaderSource& source) {
        auto rawShaderOpt = source.Produce();

        if (!rawShaderOpt) return nullptr;

        auto shader = std::make_shared<Shader>();
        shader->transientCpuData = std::make_unique<CookedShader>();
        shader->transientGpuData = std::make_unique<GpuShaderData>();
        shader->cpuData.identifier = rawShaderOpt->identifier;

        *(shader->transientCpuData) = _cooker->Cook(std::move(rawShaderOpt).value());
        _processorPipeline->Run(*(shader->transientCpuData));
        *(shader->transientGpuData) = _converter->Convert(*(shader->transientCpuData));
        _extractor->Extract(*(shader->transientGpuData), shader->cpuData);

        return shader;
    }
}