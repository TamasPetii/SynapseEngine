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

#pragma once
#include "Engine/SynApi.h"
#include "Engine/Image/Loader/IImageLoaderRegistry.h"
#include "Engine/Image/Processor/IImageProcessorPipeline.h"
#include "Engine/Image/Converter/IImageCooker.h"
#include "Engine/Image/Converter/IGpuImageConverter.h"
#include "Engine/Image/Source/IImageSource.h"
#include "Engine/Image/Data/Gpu/GpuImage.h"
#include "Engine/Image/Data/Texture.h"

#include <memory>
#include <string>
#include <optional>

namespace Syn
{
    class SYN_API ImageBuilder
    {
    public:
        ImageBuilder(
            std::unique_ptr<IImageLoaderRegistry> registry,
            std::unique_ptr<IImageProcessorPipeline> pipeline,
            std::unique_ptr<IGpuImageConverter> converter,
            std::unique_ptr<IImageCooker> cooker
        );

        ImageBuilder(const ImageBuilder&) = delete;
        ImageBuilder& operator=(const ImageBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IImageLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IImageProcessor> processor);

        std::shared_ptr<Texture> BuildFromFile(const std::string& filePath);
        std::shared_ptr<Texture> BuildFromSource(IImageSource& source);
        IImageLoader* GetLoaderForExtension(const std::string& ext) const { return _registry->GetLoaderForExtension(ext); }
    private:
        std::unique_ptr<IImageLoaderRegistry> _registry;
        std::unique_ptr<IImageProcessorPipeline> _pipeline;
        std::unique_ptr<IGpuImageConverter> _converter;
        std::unique_ptr<IImageCooker> _cooker;
    };
}