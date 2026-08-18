#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Environment/Data/EnvironmentData.h"
#include "Engine/Environment/Uploader/IEnvironmentUploader.h"
#include <functional>

namespace Syn {
    class IImageSource;

    struct EnvironmentManagerCallbacks {
        std::function<void(uint32_t, VkImageView)> updateCubeTexture;
        std::function<uint32_t(const std::string&)> getSamplerIndex;
        std::function<uint32_t(const std::string&, std::function<std::unique_ptr<IImageSource>()>)> loadProceduralImage;
        std::function<uint32_t(const std::string&)> loadImageSync;
        std::function<uint32_t(const std::string&)> loadImageAsync;
        std::function<void(uint32_t)> waitForImage;
        std::function<std::shared_ptr<Texture>(uint32_t)> getImageResource;
    };

    class SYN_API EnvironmentManager : public AddressResourceManager<Environment, EnvironmentData> {
    public:
        EnvironmentManager(
            uint32_t framesInFlight,
            std::shared_ptr<ImageBuilder> imageBuilder,
            std::unique_ptr<IEnvironmentUploader> uploader,
            EnvironmentManagerCallbacks callbacks);

        ~EnvironmentManager() override = default;

        uint32_t LoadEnvironmentSync(const std::string& filePath);
        uint32_t LoadEnvironmentAsync(const std::string& filePath);

        uint32_t GetBrdfLutId() const { return _brdfLutId; }
    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
        std::shared_ptr<Environment> CreateEnvironmentResource(uint32_t skyTexId);
    private:
        EnvironmentManagerCallbacks _callbacks;
        std::shared_ptr<ImageBuilder> _imageBuilder;
        std::unique_ptr<IEnvironmentUploader> _uploader;
        uint32_t _brdfLutId = UINT32_MAX;
    };
}