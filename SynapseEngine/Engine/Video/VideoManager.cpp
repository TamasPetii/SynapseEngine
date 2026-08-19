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

#include "VideoManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Vk/Core/Device.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Vk/Rendering/GpuUploader.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"

namespace Syn
{
    VideoManager::VideoManager(uint32_t framesInFlight, std::shared_ptr<VideoBuilder> builder, VideoManagerCallbacks callbacks)
        : AddressResourceManager<VideoStreamState, uint32_t>(framesInFlight, 64, 16, 32),
        _callbacks(std::move(callbacks)),
        _builder(builder),
        _isRunning(true)
    {
        _streamingThread = std::thread(&VideoManager::StreamingThreadLoop, this);
    }

    VideoManager::~VideoManager() {
        _isRunning = false;
        if (_streamingThread.joinable()) {
            _streamingThread.join();
        }
    }

    uint32_t VideoManager::LoadVideoAsync(const std::string& filePath) {
        return this->InternalLoadAsync(filePath, [this, filePath]() {
            auto state = std::make_shared<VideoStreamState>();
            state->source = _builder->CreateSourceFromFile(filePath);
            state->video = std::make_shared<Video>();

            if (state->source) {
                VideoInfo info = state->source->GetInfo();
                state->video->info = info;
                state->converter = _builder->CreateConverter(info);
                state->uploader = _builder->CreateUploader(info);
            }

            return state;
            });
    }

    uint32_t VideoManager::LoadVideoFromNetworkAsync(const std::string& url) {
        return this->InternalLoadAsync(url, [this, url]() {
            auto state = std::make_shared<VideoStreamState>();
            state->source = _builder->CreateSourceFromNetwork(url);
            state->video = std::make_shared<Video>();

            if (state->source) {
                VideoInfo info = state->source->GetInfo();
                state->video->info = info;
                state->converter = _builder->CreateConverter(info);
                state->uploader = _builder->CreateUploader(info);
            }

            return state;
            });
    }

    void VideoManager::StartGpuUpload(EntryType& entry) {
        uint32_t entryId = this->_pathToId.at(entry.path);
        this->SetResourceState(entryId, ResourceState::Ready);
        this->MarkDirty(entryId);
    }

    void VideoManager::FinalizeResource(EntryType& entry) {}

    void VideoManager::FlushDirtyResources() {
        std::vector<std::pair<uint32_t, uint32_t>> addressUpdates;

        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t samplerIndex = imageManager ? imageManager->GetSamplerIndex(SamplerNames::LinearClampEdge) : 0;
        uint32_t textureData = (samplerIndex & 0x7FFFFFFF);

        this->ProcessDirtyReadyEntries(
            [this, &addressUpdates, textureData](uint32_t index, const EntryType& entry) {
                if (!entry.resource->video || !entry.resource->video->image) return;

                auto targetImage = entry.resource->video->convertedImage ?
                    entry.resource->video->convertedImage :
                    entry.resource->video->image;

                if (!targetImage) return;

                if (_callbacks.updateVideoTexture) {
                    _callbacks.updateVideoTexture(index, targetImage->GetView());
                }

                addressUpdates.push_back({ index, textureData });
            }
        );

        if (!addressUpdates.empty()) {
            this->WriteAddresses(addressUpdates);
        }
    }

    void VideoManager::UpdateVideoBindlessBatch(std::span<const std::pair<uint32_t, VkImageView>> updates) {
        if (updates.empty()) return;

        auto imageManager = ServiceLocator::Get<ImageManager>();
        uint32_t samplerIndex = imageManager ? imageManager->GetSamplerIndex(SamplerNames::LinearClampEdge) : 0;
        uint32_t textureData = (samplerIndex & 0x7FFFFFFF);

        std::vector<std::pair<uint32_t, uint32_t>> addressUpdates;
        addressUpdates.reserve(updates.size());

        for (const auto& u : updates) {
            if (_callbacks.updateVideoTexture) {
                _callbacks.updateVideoTexture(u.first, u.second);
            }
            addressUpdates.push_back({ u.first, textureData });
        }

        this->WriteAddresses(addressUpdates);
    }

    void VideoManager::StreamingThreadLoop()
    {
        while (_isRunning) {
            std::vector<std::pair<uint32_t, std::shared_ptr<VideoStreamState>>> activeStreams;

            {
                std::lock_guard lock(this->_mutex);
                for (auto& entry : this->_entries) {
                    if (entry.state == ResourceState::Ready && entry.resource && entry.resource->isPlaying) {
                        activeStreams.push_back({ this->_pathToId.at(entry.path), entry.resource });
                    }
                }
            }

            for (auto& [streamId, stream] : activeStreams) {
                if (!stream->source || !stream->video || stream->isUploading) continue;

                if (stream->video->info.frameRate > 0.0) {
                    auto now = std::chrono::steady_clock::now();

                    if (!stream->hasStarted) {
                        stream->lastDecodeTime = now;
                        stream->hasStarted = true;
                    }

                    double elapsed = std::chrono::duration<double>(now - stream->lastDecodeTime).count();
                    double frameDuration = 1.0 / stream->video->info.frameRate;

                    if (elapsed < frameDuration) {
                        continue;
                    }

                    if (elapsed > frameDuration * 2.0) {
                        stream->lastDecodeTime = now;
                    }
                    else {
                        stream->lastDecodeTime += std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(frameDuration));
                    }
                }

                bool success = _builder->ProcessNextPacket(*(stream->source), *(stream->video), *(stream->converter));

                if (success) {
                    stream->isUploading = true;

                    auto uploader = stream->uploader.get();
                    auto videoData = stream->video;
                    auto streamState = stream;

                    Vk::GpuUploadRequest request{
                        .uploadCallback = [uploader, videoData, streamState](VkCommandBuffer cmd, Vk::GpuUploader* gpuUploader) {
                            auto result = uploader->Upload(*(videoData->transientGpuData), cmd, gpuUploader);

                            if (result.texture != nullptr) {
                                videoData->image = result.texture;
                            }

                            std::lock_guard<std::mutex> lock(streamState->stagingMutex);
                            streamState->activeStagingBuffer = std::move(result.bitstreamBuffer);
                        },
                        .onFinished = [this, streamId, streamState]() {
                            {
                                std::lock_guard<std::mutex> lock(streamState->stagingMutex);
                                streamState->activeStagingBuffer.reset();
                            }

                            streamState->isUploading = false;

                            this->SetResourceState(streamId, ResourceState::Ready);
                            this->MarkDirty(streamId);
                        },
                        .queueType = Vk::GpuQueueType::Video
                    };

                    ServiceLocator::Get<Vk::GpuUploader>()->Submit(std::move(request));

                }
                else if (stream->isLooping) {
                    stream->source->Reset();
                    stream->hasStarted = false;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}