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
#include "Engine/Video/Builder/VideoBuilder.h"
#include "Engine/Video/Uploader/IGpuVideoUploader.h"
#include "Engine/Video/Converter/IGpuVideoConverter.h"
#include "Engine/Video/Source/IVideoSource.h"
#include "Engine/Video/Data/Video.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <chrono>
#include <functional>

namespace Syn {

    struct VideoStreamState {
        std::unique_ptr<IVideoSource> source;
        std::shared_ptr<Video> video;
        std::unique_ptr<IGpuVideoConverter> converter;
        std::unique_ptr<IGpuVideoUploader> uploader;

        std::atomic<bool> isUploading = false;
        std::mutex stagingMutex;
        std::unique_ptr<Vk::Buffer> activeStagingBuffer;

        bool isPlaying = true;
        bool isLooping = true;
        bool hasStarted = false;
        std::chrono::steady_clock::time_point lastDecodeTime;
    };

    struct VideoManagerCallbacks {
        std::function<void(uint32_t, VkImageView)> updateVideoTexture;
    };

    class SYN_API VideoManager : public AddressResourceManager<VideoStreamState, uint32_t> {
    public:
        VideoManager(uint32_t framesInFlight, std::shared_ptr<VideoBuilder> builder, VideoManagerCallbacks callbacks);
        ~VideoManager() override;

        uint32_t LoadVideoAsync(const std::string& filePath);
        void UpdateVideoBindlessBatch(std::span<const std::pair<uint32_t, VkImageView>> updates);

    protected:
        void FlushDirtyResources() override;
        void StartGpuUpload(EntryType& entry) override;
        void FinalizeResource(EntryType& entry) override;

    private:
        void StreamingThreadLoop();

    private:
        VideoManagerCallbacks _callbacks;
        std::shared_ptr<VideoBuilder> _builder;

        std::thread _streamingThread;
        std::atomic<bool> _isRunning;
    };
}