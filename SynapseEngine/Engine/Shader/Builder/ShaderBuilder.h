#pragma once
#include "Engine/SynApi.h"
#include "Engine/Shader/Loader/IShaderLoaderRegistry.h"
#include "Engine/Shader/Processor/IShaderProcessorPipeline.h"
#include "Engine/Shader/Converter/IShaderCooker.h"
#include "Engine/Shader/Resolver/IShaderDependencyResolver.h"
#include "Engine/Shader/Converter/IGpuShaderConverter.h"
#include "Engine/Shader/Converter/ICpuShaderExtractor.h"
#include "Engine/Shader/Source/IShaderSource.h"
#include "Engine/Shader/Data/Shader.h"

#include <memory>
#include <string>
#include <vector>

namespace Syn {
    class SYN_API ShaderBuilder {
    public:
        ShaderBuilder(
            std::unique_ptr<IShaderLoaderRegistry> loaderRegistry,
            std::unique_ptr<IShaderProcessorPipeline> processorPipeline,
            std::unique_ptr<IShaderCooker> cooker,
            std::unique_ptr<IGpuShaderConverter> converter,
            std::unique_ptr<ICpuShaderExtractor> extractor,
            std::unique_ptr<IShaderDependencyResolver> dependencyResolver
        );

        ShaderBuilder(const ShaderBuilder&) = delete;
        ShaderBuilder& operator=(const ShaderBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IShaderLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IShaderProcessor> processor);

        std::shared_ptr<Shader> BuildFromFile(const std::string& filePath, VkShaderStageFlagBits stage, const std::vector<std::string>& defines = {});
        std::shared_ptr<Shader> BuildFromSource(IShaderSource& source);

    private:
        std::unique_ptr<IShaderLoaderRegistry> _loaderRegistry;
        std::unique_ptr<IShaderProcessorPipeline> _processorPipeline;
        std::unique_ptr<IShaderCooker> _cooker;
        std::unique_ptr<IGpuShaderConverter> _converter;
        std::unique_ptr<ICpuShaderExtractor> _extractor;
        std::unique_ptr<IShaderDependencyResolver> _dependencyResolver;
    };
}