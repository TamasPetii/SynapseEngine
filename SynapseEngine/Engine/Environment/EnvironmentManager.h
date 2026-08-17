#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Image/Builder/ImageBuilder.h"
#include "Engine/Environment/Data/EnvironmentData.h"
#include "Engine/Environment/Uploader/IEnvironmentUploader.h"
#include <functional>

namespace Syn {
    struct EnvironmentManagerCallbacks {
        std::function<void(uint32_t, VkImageView)> updateCubeTexture;
    };

    class SYN_API EnvironmentManager : public AddressResourceManager<Environment, EnvironmentData> {
    public:
        EnvironmentManager(
            uint32_t framesInFlight,
            std::shared_ptr<ImageBuilder> imageBuilder,
            std::unique_ptr<IEnvironmentUploader> uploader,
            EnvironmentManagerCallbacks callbacks);

        ~EnvironmentManager() override = default;

        uint32_t LoadEnvironmentAsync(const std::string& filePath);

    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;

    private:
        EnvironmentManagerCallbacks _callbacks;
        std::shared_ptr<ImageBuilder> _imageBuilder;
        std::unique_ptr<IEnvironmentUploader> _uploader;
    };
}