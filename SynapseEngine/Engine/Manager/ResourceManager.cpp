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

#include "ResourceManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/Loader/ShaderLoaderRegistry.h"
#include "Engine/Shader/Loader/GlslShaderLoader.h"
#include "Engine/Shader/Compiler/ShaderCompilerRegistry.h"
#include "Engine/Shader/Compiler/Shaderc/ShadercShaderCompiler.h"
#include "Engine/Shader/Reflector/ShaderReflectorRegistry.h"
#include "Engine/Shader/Reflector/SpirvShaderReflector.h"
#include "Engine/Shader/Processor/ShaderProcessorPipeline.h"
#include "Engine/Shader/Converter/DefaultShaderCooker.h"
#include "Engine/Shader/Converter/DefaultGpuShaderConverter.h"
#include "Engine/Shader/Converter/DefaultCpuShaderExtractor.h"
#include "Engine/Shader/Resolver/DefaultShaderDependencyResolver.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/DefaultGpuModelUploader.h"
#include "Engine/Mesh/Converter/DefaultModelCooker.h"
#include "Engine/Mesh/Converter/DefaultGpuModelConverter.h"
#include "Engine/Mesh/Converter/DefaultCpuModelExtractor.h"
#include "Engine/Animation/Converter/DefaultCpuAnimationExtractor.h"
#include "Engine/Image/Converter/DefaultCpuImageExtractor.h"
#include "Engine/Mesh/Loader/MeshLoaders.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/Processor/MeshProcessor/MeshProcessors.h" 
#include "Engine/Image/Loader/ImageLoaderRegistry.h"
#include "Engine/Image/Source/Memory/MemoryImageSource.h"
#include "Engine/Image/Processor/ImageProcessorPipeline.h"
#include "Engine/Image/Converter/DefaultGpuImageConverter.h"
#include "Engine/Image/Converter/DefaultImageCooker.h"
#include "Engine/Image/Loader/StbImageLoader.h"
#include "Engine/Image/Loader/GliImageLoader.h"
#include "Engine/Image/Loader/SvgImageLoader.h"
#include "Engine/Image/Loader/HdriImageLoader.h"
#include "Engine/Image/Uploader/DefaultGpuImageUploader.h"
#include "Engine/Animation/Loader/AnimationLoaderRegistry.h"
#include "Engine/Animation/Processor/AnimationProcessorPipeline.h"
#include "Engine/Animation/Converter/DefaultGpuAnimationConverter.h"
#include "Engine/Animation/Converter/DefaultAnimationCooker.h"
#include "Engine/Animation/Loader/AssimpAnimationLoader.h"
#include "Engine/Animation/Loader/TinyGltfAnimationLoader.h"
#include "Engine/Animation/Processor/Geometry/AnimationBakeProcessor.h"
#include "Engine/Animation/Processor/Geometry/AnimationColliderProcessor.h"
#include "Engine/Animation/Uploader/DefaultGpuAnimationUploader.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/CpuModelProcessorPipeline.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/BatchedIndicesProcessor.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/VertexWeldingProcessor.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/VertexTransformProcessor.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/MemoryCleanupProcessor.h"
#include "Engine/Audio/Loader/DefaultAudioLoaderRegistry.h"
#include "Engine/Audio/Loader/MiniAudioLoader.h"
#include "Engine/Audio/Processor/DefaultAudioProcessorPipeline.h"
#include "Engine/Audio/Converter/DefaultAudioCooker.h"
#include "Engine/Audio/Converter/DefaultCpuAudioExtractor.h"
#include "Engine/Audio/Processor/AudioWaveformProcessor.h"
#include "Engine/Video/Loader/VideoLoaderRegistry.h"
#include "Engine/Video/Loader/FFmpeg/FFmpegVideoLoader.h"
#include "Engine/Video/Processor/VideoProcessorPipeline.h"
#include "Engine/Video/Processor/AnnexBVideoProcessor.h"
#include "Engine/Video/Processor/VulkanAnnexBVideoProcessor.h"
#include "Engine/Video/Converter/DefaultVideoCooker.h"
#include "Engine/Video/Converter/DefaultGpuVideoConverter.h"
#include "Engine/Video/Uploader/VulkanGpuVideoUploader.h"
#include "Engine/Video/Converter/FFmpegCpuVideoConverter.h"
#include "Engine/Video/Uploader/CpuPixelVideoUploader.h"
#include "Engine/Video/Parser/H264ExtradataParser.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Environment/Uploader/DefaultEnvironmentUploader.h"

namespace Syn {
    ResourceManager::ResourceManager(uint32_t framesInFlight) : _framesInFlight(framesInFlight) {
        InitDescriptorManager();
        InitShaderManager();
        InitPreviewManager();
        InitImageManager();
        InitMaterialManager();
        InitModelManager();
        InitAnimationManager();
        InitAudioManager();
        InitVideoManager(true);
        InitEnvironmentManager();
    }

    void ResourceManager::InitDescriptorManager() {
        _descriptorManager = std::make_unique<DescriptorManager>(_framesInFlight);
        ServiceLocator::Provide<DescriptorManager>(_descriptorManager.get());
    }

    void ResourceManager::InitPreviewManager() {
        _previewManager = std::make_unique<PreviewManager>(2048, 128);
        ServiceLocator::Provide<PreviewManager>(_previewManager.get());
    }

    void ResourceManager::InitShaderManager() {
        auto loaderRegistry = std::make_unique<ShaderLoaderRegistry>();
        loaderRegistry->Register(std::make_shared<GlslShaderLoader>(), 1);

        auto compilerRegistry = std::make_unique<ShaderCompilerRegistry>();
        compilerRegistry->Register(ShaderLanguage::GLSL, std::make_shared<ShadercShaderCompiler>());

        auto reflectorRegistry = std::make_unique<ShaderReflectorRegistry>();
        reflectorRegistry->Register(ShaderBytecodeFormat::SPIRV, std::make_shared<SpirvShaderReflector>());

        _shaderBuilder = std::make_shared<ShaderBuilder>(
            std::move(loaderRegistry),
            std::make_unique<ShaderProcessorPipeline>(),
            std::make_unique<DefaultShaderCooker>(),
            std::make_unique<DefaultGpuShaderConverter>(std::move(compilerRegistry)),
            std::make_unique<DefaultCpuShaderExtractor>(std::move(reflectorRegistry)),
            std::make_unique<DefaultShaderDependencyResolver>()
        );

        ServiceLocator::Provide<ShaderBuilder>(_shaderBuilder.get());

        _shaderManager = std::make_unique<ShaderManager>(_shaderBuilder);
        ServiceLocator::Provide<ShaderManager>(_shaderManager.get());
    }

    void ResourceManager::InitImageManager() {
        _imageBuilder = std::make_shared<ImageBuilder>(
            std::make_unique<ImageLoaderRegistry>(),
            std::make_unique<ImageProcessorPipeline>(),
            std::make_unique<DefaultGpuImageConverter>(),
            std::make_unique<DefaultImageCooker>()
        );

        _imageBuilder->RegisterLoader(std::make_shared<StbImageLoader>(), 1);
        _imageBuilder->RegisterLoader(std::make_shared<GliImageLoader>(), 1);
        _imageBuilder->RegisterLoader(std::make_shared<SvgImageLoader>(), 1);
        _imageBuilder->RegisterLoader(std::make_shared<HdriImageLoader>(), 1);

        ServiceLocator::Provide<ImageBuilder>(_imageBuilder.get());

        ImageManagerCallbacks callbacks{
            .registerSampler = [this](uint32_t index, VkSampler sampler) {
                _descriptorManager->WriteSampler(index, sampler);
            },
            .fillDefaultTexture = [this](VkImageView view) {
                _descriptorManager->FillTextures(view);
            },
            .updateTexture = [this](uint32_t index, VkImageView view) {
                _descriptorManager->WriteTexture(index, view);
            },
            .notifyMaterialManager = [](uint32_t imageId) {
                auto matManager = ServiceLocator::Get<MaterialManager>();
                if (matManager) {
                    matManager->NotifyImageReady(imageId);
                }
            }
        };

        _imageManager = std::make_unique<ImageManager>(
            _framesInFlight,
            _imageBuilder,
            std::make_unique<DefaultGpuImageUploader>(),
            std::make_unique<DefaultCpuImageExtractor>(),
            callbacks
        );

        ServiceLocator::Provide<ImageManager>(_imageManager.get());
    }

    void ResourceManager::InitMaterialManager() {
        MaterialManagerCallbacks callbacks{
            .textureLoad = [this](const TexturePayload& payload) -> uint32_t {
                if (payload.IsEmbedded()) {
                    size_t hash = payload.embeddedData.size();

                    if (hash > 0) {
                        hash ^= (payload.embeddedData.front() << 16) | payload.embeddedData.back();
                    }

                    std::string uniqueName = "Embedded_" + std::to_string(hash) + "_" + payload.path;
                    std::string ext = payload.formatHint.empty() ? "" : "." + payload.formatHint;
                    IImageLoader* loader = _imageBuilder->GetLoaderForExtension(ext);

                    return _imageManager->LoadImageFromSourceAsync(uniqueName, [payload, loader]() {
                        return std::make_unique<MemoryImageSource>(payload, loader);
                        });
                }
                else {
                    return _imageManager->LoadImageAsync(payload.path);
                }
            },
            .previewAllocate = [this](uint32_t id) {
                if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Material, id);
            },
            .previewMarkDirty = [this](uint32_t id) {
                if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Material, id);
            },
            .materialReadyOrChanged = [](uint32_t materialId) {
                auto modelManager = ServiceLocator::Get<ModelManager>();
                if (modelManager) {
                    modelManager->NotifyMaterialReady(materialId);
                }
            }
        };

        _materialManager = std::make_unique<MaterialManager>(
            _framesInFlight,
            callbacks
        );

        ServiceLocator::Provide<MaterialManager>(_materialManager.get());
    }

    void ResourceManager::InitModelManager() {
        _staticMeshBuilder = std::make_shared<StaticMeshBuilder>(
            std::make_unique<MeshLoaderRegistry>(),
            std::make_unique<MeshProcessorPipeline>(),
            std::make_unique<DefaultGpuModelConverter>(),
            std::make_unique<DefaultModelCooker>(),
            std::make_unique<DefaultCpuModelExtractor>(),
            std::make_unique<CpuModelProcessorPipeline>()
        );

        _staticMeshBuilder->RegisterLoader(std::make_shared<AssimpMeshLoader>(), 1);
        _staticMeshBuilder->RegisterLoader(std::make_shared<TinyGltfLoader>(), 0);

        _staticMeshBuilder->RegisterMeshProcessor(std::make_unique<NormalProcessor>());
        _staticMeshBuilder->RegisterMeshProcessor(std::make_unique<TangentProcessor>());
        _staticMeshBuilder->RegisterMeshProcessor(std::make_unique<MeshoptimizerLodProcessor>());
        _staticMeshBuilder->RegisterMeshProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());
        _staticMeshBuilder->RegisterMeshProcessor(std::make_unique<ColliderProcessor>());

        _staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<BatchedIndicesProcessor>());
        _staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<VertexTransformProcessor>());
        _staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<VertexWeldingProcessor>());
        _staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<MemoryCleanupProcessor>());

        ServiceLocator::Provide<StaticMeshBuilder>(_staticMeshBuilder.get());

        ModelManagerCallbacks callbacks{
            .materialLoad = [this](const std::string& name, const MaterialInfo& info) -> uint32_t {
                return _materialManager->LoadMaterialSync(name, info);
            },
            .previewAllocate = [this](uint32_t id) {
                if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Model, id);
            },
            .previewMarkDirty = [this](uint32_t id) {
                if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Model, id);
            }
        };

        _modelManager = std::make_unique<ModelManager>(
            _framesInFlight,
            _staticMeshBuilder,
            std::make_unique<DefaultGpuModelUploader>(),
            callbacks
        );

        ServiceLocator::Provide<ModelManager>(_modelManager.get());

        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Sphere, []() { return MeshFactory::CreateSphere(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxySphere, []() { return MeshFactory::CreateProxySphere(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cube, []() { return MeshFactory::CreateCube(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Quad, []() { return MeshFactory::CreateQuad(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ScreenQuad, []() { return MeshFactory::CreateScreenQuad(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cylinder, []() { return MeshFactory::CreateCylinder(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cone, []() { return MeshFactory::CreateCone(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyCone, []() { return MeshFactory::CreateProxyCone(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Capsule, []() { return MeshFactory::CreateCapsule(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Hemisphere, []() { return MeshFactory::CreateHemisphere(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Pyramid, []() { return MeshFactory::CreatePyramid(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyPyramid, []() { return MeshFactory::CreateProxyPyramid(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Grid, []() { return MeshFactory::CreateGrid(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Torus, []() { return MeshFactory::CreateTorus(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::IcoSphere, []() { return MeshFactory::CreateIcoSphere(); });
        _modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyIcoSphere, []() { return MeshFactory::CreateProxyIcoSphere(); });
    }

    void ResourceManager::InitAnimationManager() {
        _animationBuilder = std::make_shared<AnimationBuilder>(
            std::make_unique<AnimationLoaderRegistry>(),
            std::make_unique<AnimationProcessorPipeline>(),
            std::make_unique<DefaultGpuAnimationConverter>(),
            std::make_unique<DefaultAnimationCooker>()
        );

        _animationBuilder->RegisterLoader(std::make_shared<AssimpAnimationLoader>(), 1);
        _animationBuilder->RegisterLoader(std::make_shared<TinyGltfAnimationLoader>(), 0);
        _animationBuilder->RegisterProcessor(std::make_unique<AnimationBakeProcessor>());
        _animationBuilder->RegisterProcessor(std::make_unique<AnimationColliderProcessor>());

        ServiceLocator::Provide<AnimationBuilder>(_animationBuilder.get());

        AnimationManagerCallbacks callbacks{
            .previewAllocate = [this](uint32_t id) {
                if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Animation, id);
            },
            .previewMarkDirty = [this](uint32_t id) {
                if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Animation, id);
            }
        };

        _animationManager = std::make_unique<AnimationManager>(
            _framesInFlight,
            _animationBuilder,
            std::make_unique<DefaultGpuAnimationUploader>(),
            std::make_unique<DefaultCpuAnimationExtractor>(),
            callbacks
        );

        ServiceLocator::Provide<AnimationManager>(_animationManager.get());
    }

    void ResourceManager::InitAudioManager() {
        _audioBuilder = std::make_shared<AudioBuilder>(
            std::make_unique<DefaultAudioLoaderRegistry>(),
            std::make_unique<DefaultAudioProcessorPipeline>(),
            std::make_unique<DefaultAudioCooker>(),
            std::make_unique<DefaultCpuAudioExtractor>()
        );

        _audioBuilder->RegisterLoader(std::make_shared<MiniAudioLoader>(), 1);
        _audioBuilder->RegisterProcessor(std::make_unique<AudioWaveformProcessor>());

        ServiceLocator::Provide<AudioBuilder>(_audioBuilder.get());

        _audioManager = std::make_unique<AudioManager>(
            _audioBuilder,
            [this](uint32_t id) {
                if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Audio, id);
            },
            [this](uint32_t id) {
                if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Audio, id);
            }
        );

        ServiceLocator::Provide<AudioManager>(_audioManager.get());
    }

    void ResourceManager::InitVideoManager(bool useGpuDecoding) {
        auto loaderRegistry = std::make_unique<VideoLoaderRegistry>();
        loaderRegistry->Register(std::make_shared<FFmpegVideoLoader>(), 1);

        auto pipeline = std::make_unique<VideoProcessorPipeline>();

        VideoConverterFactory converterFactory;
        VideoUploaderFactory uploaderFactory;

        uint32_t bufferCount = _framesInFlight + 2;

        auto h264Parser = std::make_shared<H264ExtradataParser>();

        if (useGpuDecoding) {
            pipeline->AddProcessor(std::make_unique<VulkanAnnexBVideoProcessor>());

            converterFactory = [](const VideoInfo& info) {
                return std::make_unique<DefaultGpuVideoConverter>();
                };
            uploaderFactory = [h264Parser, bufferCount](const VideoInfo& info) {
                return std::make_unique<VulkanGpuVideoUploader>(info.width, info.height, bufferCount, info.extradata, h264Parser);
                };
        }
        else {
            pipeline->AddProcessor(std::make_unique<AnnexBVideoProcessor>());

            converterFactory = [](const VideoInfo& info) {
                return std::make_unique<FFmpegCpuVideoConverter>(AV_CODEC_ID_H264, info.width, info.height, info.extradata);
                };
            uploaderFactory = [bufferCount](const VideoInfo& info) {
                return std::make_unique<CpuPixelVideoUploader>(info.width, info.height, bufferCount);
                };
        }

        _videoBuilder = std::make_shared<VideoBuilder>(
            std::move(loaderRegistry),
            std::move(pipeline),
            std::move(converterFactory),
            std::move(uploaderFactory),
            std::make_unique<DefaultVideoCooker>(),
            h264Parser
        );

        ServiceLocator::Provide<VideoBuilder>(_videoBuilder.get());

        VideoManagerCallbacks callbacks{
            .updateVideoTexture = [this](uint32_t index, VkImageView view) {
                _descriptorManager->WriteVideoTexture(index, view);
            }
        };

        _videoManager = std::make_unique<VideoManager>(
            _framesInFlight,
            _videoBuilder,
            callbacks
        );
        ServiceLocator::Provide<VideoManager>(_videoManager.get());
    }

    void ResourceManager::InitEnvironmentManager() {
        EnvironmentManagerCallbacks callbacks{
            .updateCubeTexture = [this](uint32_t index, VkImageView view) {
                _descriptorManager->WriteCubeTexture(index, view);
            },
            .getSamplerIndex = [this](const std::string& name) {
                return _imageManager->GetSamplerIndex(name);
            },
            .loadProceduralImage = [this](const std::string& name, std::function<std::unique_ptr<IImageSource>()> factory) {
                return _imageManager->LoadImageFromSourceAsync(name, factory);
            },
            .loadImageSync = [this](const std::string& path) {
                return _imageManager->LoadImageSync(path);
            },
            .loadImageAsync = [this](const std::string& path) {
                return _imageManager->LoadImageAsync(path);
            },
            .waitForImage = [this](uint32_t id) {
                _imageManager->WaitForResource(id);
            },
            .getImageResource = [this](uint32_t id) {
                return _imageManager->GetResource(id);
            }
        };

        _environmentManager = std::make_unique<EnvironmentManager>(
            _framesInFlight,
            _imageBuilder,
            std::make_unique<DefaultEnvironmentUploader>(),
            callbacks
        );

        ServiceLocator::Provide<EnvironmentManager>(_environmentManager.get());
    }

    ResourceManager::~ResourceManager() {
        ServiceLocator::Provide<ShaderManager>(nullptr);
        ServiceLocator::Provide<ResourceManager>(nullptr);
        ServiceLocator::Provide<StaticMeshBuilder>(nullptr);
        ServiceLocator::Provide<ModelManager>(nullptr);
        ServiceLocator::Provide<ImageBuilder>(nullptr);
        ServiceLocator::Provide<ImageManager>(nullptr);
        ServiceLocator::Provide<MaterialManager>(nullptr);
        ServiceLocator::Provide<AudioBuilder>(nullptr);
        ServiceLocator::Provide<AudioManager>(nullptr);
        ServiceLocator::Provide<EnvironmentManager>(nullptr);
        ServiceLocator::Provide<DescriptorManager>(nullptr);
    }
}