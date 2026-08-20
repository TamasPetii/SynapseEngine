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

#include "VideoBuilder.h"
#include "Engine/Video/Source/File/FileVideoSource.h"
#include "Engine/Video/Source/Network/NetworkVideoSource.h" 
#include <filesystem>

namespace Syn
{
    VideoBuilder::VideoBuilder(
        std::unique_ptr<IVideoLoaderRegistry> registry,
        std::unique_ptr<IVideoProcessorPipeline> pipeline,
        VideoConverterFactory converterFactory,
        VideoUploaderFactory uploaderFactory,
        std::unique_ptr<IVideoCooker> cooker,
        std::shared_ptr<IH264ExtradataParser> h264Parser) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _converterFactory(std::move(converterFactory)),
        _uploaderFactory(std::move(uploaderFactory)),
        _cooker(std::move(cooker)),
        _h264Parser(std::move(h264Parser))
    {}

    std::unique_ptr<IGpuVideoConverter> VideoBuilder::CreateConverter(const VideoInfo& info) const {
        return _converterFactory(info);
    }

    std::unique_ptr<IGpuVideoUploader> VideoBuilder::CreateUploader(const VideoInfo& info) const {
        return _uploaderFactory(info);
    }

    void VideoBuilder::RegisterLoader(std::shared_ptr<IVideoLoader> loader, int priority)
    {
        _registry->Register(loader, priority);
    }

    void VideoBuilder::RegisterProcessor(std::unique_ptr<IVideoProcessor> processor)
    {
        _pipeline->AddProcessor(std::move(processor));
    }

    std::unique_ptr<IVideoSource> VideoBuilder::CreateSourceFromFile(const std::string& filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();
        IVideoLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader)
            return nullptr;

        return std::make_unique<FileVideoSource>(filePath, loader);
    }

    std::unique_ptr<IVideoSource> VideoBuilder::CreateSourceFromNetwork(const std::string& url)
    {
        std::string cleanUrl = url;
        size_t queryPos = cleanUrl.find('?');
        if (queryPos != std::string::npos) {
            cleanUrl = cleanUrl.substr(0, queryPos);
        }

        std::string ext = std::filesystem::path(cleanUrl).extension().string();

        if (ext.empty()) {
            ext = ".mp4";
        }

        IVideoLoader* loader = _registry->GetLoaderForExtension(ext);

        if (!loader) {
            return nullptr;
        }

        return std::make_unique<NetworkVideoSource>(url, loader);
    }

    bool VideoBuilder::ProcessNextPacket(IVideoSource& source, Video& outVideo, IGpuVideoConverter& converter)
    {
        auto rawPacketOpt = source.ReadNextPacket();
        if (!rawPacketOpt) return false;

        if (!outVideo.transientCpuData) outVideo.transientCpuData = std::make_unique<CookedVideoPacket>();
        if (!outVideo.transientGpuData) outVideo.transientGpuData = std::make_unique<GpuVideoPacket>();

        *(outVideo.transientCpuData) = _cooker->Cook(std::move(rawPacketOpt).value());
        _pipeline->Run(*(outVideo.transientCpuData));
        *(outVideo.transientGpuData) = converter.Convert(*(outVideo.transientCpuData));

        return true;
    }
}