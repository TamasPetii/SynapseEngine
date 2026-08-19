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
#include "Engine/Video/Loader/IVideoLoaderRegistry.h"
#include "Engine/Video/Processor/IVideoProcessorPipeline.h"
#include "Engine/Video/Converter/IVideoCooker.h"
#include "Engine/Video/Converter/IGpuVideoConverter.h"
#include "Engine/Video/Uploader/IGpuVideoUploader.h"
#include "Engine/Video/Source/IVideoSource.h"
#include "Engine/Video/Data/Video.h"
#include "Engine/Video/Parser/IH264ExtradataParser.h"

#include <memory>
#include <string>
#include <functional>

namespace Syn
{
    using VideoConverterFactory = std::function<std::unique_ptr<IGpuVideoConverter>(const VideoInfo&)>;
    using VideoUploaderFactory = std::function<std::unique_ptr<IGpuVideoUploader>(const VideoInfo&)>;

    class SYN_API VideoBuilder
    {
    public:
        VideoBuilder(
            std::unique_ptr<IVideoLoaderRegistry> registry,
            std::unique_ptr<IVideoProcessorPipeline> pipeline,
            VideoConverterFactory converterFactory,
            VideoUploaderFactory uploaderFactory,
            std::unique_ptr<IVideoCooker> cooker,
            std::shared_ptr<IH264ExtradataParser> _h264Parser
        );

        VideoBuilder(const VideoBuilder&) = delete;
        VideoBuilder& operator=(const VideoBuilder&) = delete;

        void RegisterLoader(std::shared_ptr<IVideoLoader> loader, int priority = 0);
        void RegisterProcessor(std::unique_ptr<IVideoProcessor> processor);

        std::unique_ptr<IVideoSource> CreateSourceFromFile(const std::string& filePath);
        std::unique_ptr<IVideoSource> CreateSourceFromNetwork(const std::string& url);

        std::unique_ptr<IGpuVideoConverter> CreateConverter(const VideoInfo& info) const;
        std::unique_ptr<IGpuVideoUploader> CreateUploader(const VideoInfo& info) const;

        bool ProcessNextPacket(IVideoSource& source, Video& outVideo, IGpuVideoConverter& converter);
        IVideoLoader* GetLoaderForExtension(const std::string& ext) const { return _registry->GetLoaderForExtension(ext); }
    private:
        std::unique_ptr<IVideoLoaderRegistry> _registry;
        std::unique_ptr<IVideoProcessorPipeline> _pipeline;
        VideoConverterFactory _converterFactory;
        VideoUploaderFactory _uploaderFactory;
        std::unique_ptr<IVideoCooker> _cooker;
		std::shared_ptr<IH264ExtradataParser> _h264Parser;
    };
}