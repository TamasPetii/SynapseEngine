#include "VideoBuilder.h"
#include "Engine/Video/Source/File/FileVideoSource.h"
#include <filesystem>

namespace Syn
{
    VideoBuilder::VideoBuilder(
        std::unique_ptr<IVideoLoaderRegistry> registry,
        std::unique_ptr<IVideoProcessorPipeline> pipeline,
        VideoConverterFactory converterFactory,
        VideoUploaderFactory uploaderFactory,
        std::unique_ptr<IVideoCooker> cooker) :
        _registry(std::move(registry)),
        _pipeline(std::move(pipeline)),
        _converterFactory(std::move(converterFactory)),
        _uploaderFactory(std::move(uploaderFactory)),
        _cooker(std::move(cooker))
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