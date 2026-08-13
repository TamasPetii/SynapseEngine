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

#include "ImageBuilder.h"
#include "Engine/Image/Source/File/FileImageSource.h"

namespace Syn
{
    ImageBuilder::ImageBuilder(
        std::unique_ptr<IImageLoaderRegistry> registry,
        std::unique_ptr<IImageProcessorPipeline> pipeline,
        std::unique_ptr<IGpuImageConverter> converter,
        std::unique_ptr<IImageCooker> cooker) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _converter(std::move(converter)),
        _cooker(std::move(cooker))
    {}

    void ImageBuilder::RegisterLoader(std::shared_ptr<IImageLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void ImageBuilder::RegisterProcessor(std::unique_ptr<IImageProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::shared_ptr<Texture> ImageBuilder::BuildFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IImageLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        FileImageSource source(filePath, loader);
        return BuildFromSource(source);
    }

    std::shared_ptr<Texture> ImageBuilder::BuildFromSource(IImageSource& source)
    {
        auto rawTextureOpt = source.Produce();

        if (!rawTextureOpt)
            return nullptr;

		auto texture = std::make_shared<Texture>();
        texture->transientCpuData = std::make_unique<CookedImage>();
        texture->transientGpuData = std::make_unique<GpuImage>();

        *(texture->transientCpuData) = _cooker->Cook(std::move(rawTextureOpt).value());
        _pipeline->Run(*(texture->transientCpuData));
        *(texture->transientGpuData) = _converter->Convert(*(texture->transientCpuData));

        return texture;
    }
}