#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/BaseResourceManager.h"
#include "Engine/Animation/Builder/AnimationBuilder.h"
#include "Engine/Animation/Uploader/IGpuAnimationUploader.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Utils/WindowedBuffer.h"

namespace Syn {

    class SYN_API AnimationManager : public BaseResourceManager<Animation> {
    public:
        static constexpr uint32_t MAX_ANIMATIONS = 10000;

        AnimationManager(std::shared_ptr<AnimationBuilder> builder, std::unique_ptr<IGpuAnimationUploader> uploader);
        ~AnimationManager() = default;

        uint32_t LoadAnimationAsync(const std::string& filePath, uint32_t baseModelId);
        uint32_t LoadAnimationSync(const std::string& filePath, uint32_t baseModelId);

        Vk::Buffer* GetAnimationAddressBuffer() const { return _animationAddressBuffer.get(); }
    protected:
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;
    private:
        std::shared_ptr<AnimationBuilder> _builder;
        std::unique_ptr<IGpuAnimationUploader> _uploader;
        std::unique_ptr<Vk::Buffer> _animationAddressBuffer;
    };
}