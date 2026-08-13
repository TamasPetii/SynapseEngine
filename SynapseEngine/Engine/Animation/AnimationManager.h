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
#include "Engine/Manager/AddressResourceManager.h"
#include "Engine/Animation/Builder/AnimationBuilder.h"
#include "Engine/Animation/Uploader/IGpuAnimationUploader.h"
#include "Engine/Animation/Converter/ICpuAnimationExtractor.h"

#include "Engine/Vk/Core/ThreadSafeQueue.h"
#include "Engine/Vk/Command/CommandPool.h"
#include "Engine/Utils/WindowedBuffer.h"

namespace Syn {

    using PreviewAllocateCallback = std::function<void(uint32_t resourceId)>;
    using PreviewMarkDirtyCallback = std::function<void(uint32_t resourceId)>;

    class SYN_API AnimationManager : public AddressResourceManager<Animation, GpuAnimationAddresses> {
    public:
        AnimationManager(uint32_t framesInFlight, 
            std::shared_ptr<AnimationBuilder> builder, 
            std::unique_ptr<IGpuAnimationUploader> uploader,
            std::unique_ptr<ICpuAnimationExtractor> cpuExtractor,
            PreviewAllocateCallback previewAllocateCallback = nullptr,
            PreviewMarkDirtyCallback previewMarkDirtyCallback = nullptr);
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

        PreviewAllocateCallback _previewAllocateCallback;
        PreviewMarkDirtyCallback _previewMarkDirtyCallback;
    };
}