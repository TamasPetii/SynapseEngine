#pragma once
#include "Engine/SynApi.h"
#include "BaseResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Image/Uploader/IGpuImageUploader.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"

namespace Syn {

    using ImageSourceFactory = std::function<std::unique_ptr<IImageSource>()>;

    class SYN_API ImageManager : public BaseResourceManager<Texture> {
    public:
        ImageManager(std::shared_ptr<ImageBuilder> builder, std::unique_ptr<IGpuImageUploader> uploader);
        ~ImageManager() = default;

        uint32_t LoadImageAsync(const std::string& filePath);
        uint32_t LoadImageFromSourceAsync(const std::string& name, ImageSourceFactory factory);
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        std::shared_ptr<ImageBuilder> _builder;
        std::unique_ptr<IGpuImageUploader> _uploader;
    };
}