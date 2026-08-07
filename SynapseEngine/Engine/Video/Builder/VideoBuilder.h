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