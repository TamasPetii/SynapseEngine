#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Animation/Builder/AnimationBuilder.h"
#include "Engine/Animation/Uploader/IGpuAnimationUploader.h"
#include "Engine/Animation/Converter/ICpuAnimationExtractor.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Utils/WindowedBuffer.h"

namespace Syn {

    class SYN_API AnimationManager : public AddressResourceManager<Animation, GpuAnimationAddresses> {
    public:
        AnimationManager(uint32_t framesInFlight, 
            std::shared_ptr<AnimationBuilder> builder, 
            std::unique_ptr<IGpuAnimationUploader> uploader,
            std::unique_ptr<ICpuAnimationExtractor> cpuExtractor);
        ~AnimationManager() = default;

        uint32_t LoadAnimationAsync(const std::string& filePath, uint32_t baseModelId);
        uint32_t LoadAnimationSync(const std::string& filePath, uint32_t baseModelId);
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
        void FlushDirtyResources() override;
    private:
        std::shared_ptr<AnimationBuilder> _builder;
        std::unique_ptr<IGpuAnimationUploader> _uploader;
        std::unique_ptr<ICpuAnimationExtractor> _cpuExtractor;
    };
}