#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Image/Image.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <glm/glm.hpp>

namespace Syn {

    enum class PreviewResourceType : uint32_t {
        Material = 0,
        Model = 1,
        Image = 2
    };

    class SYN_API PreviewManager {
    public:
        using AtlasResizedCallback = std::function<void(VkImageView, VkSampler)>;

        PreviewManager(uint32_t initialResolution = 1024, uint32_t tileSize = 64);
        virtual ~PreviewManager() = default;

        bool AllocateTile(PreviewResourceType type, uint32_t resourceId);
        void FreeTile(PreviewResourceType type, uint32_t resourceId);
        bool HasTile(PreviewResourceType type, uint32_t resourceId) const;

        void MarkDirty(PreviewResourceType type, uint32_t resourceId);
        std::vector<uint32_t> GetDirtyResources(PreviewResourceType type);
        void ClearDirtyResources(PreviewResourceType type);
        void ClearAllDirtyResources();

        void GetViewportAndScissor(PreviewResourceType type, uint32_t resourceId, VkViewport& outViewport, VkRect2D& outScissor) const;
        void GetNormalizedUVs(PreviewResourceType type, uint32_t resourceId, glm::vec2& outUv0, glm::vec2& outUv1) const;

        Vk::Image* GetAtlasImage() const { return _atlasImage.get(); }
        Vk::Image* GetAtlasDepthImage() const { return _atlasDepthImage.get(); }
        Vk::Image* GetScratchColorImage() const { return _scratchColorImage.get(); }
        Vk::Image* GetScratchBloomImage() const { return _scratchBloomImage.get(); }

        uint32_t GetResolution() const { return _resolution; }
        std::vector<uint32_t> GetActiveResources(PreviewResourceType type) const;
    private:
        void CreateOrResizeAtlas(uint32_t newResolution);
        uint64_t GetUniqueId(PreviewResourceType type, uint32_t resourceId) const;
    protected:
        mutable std::mutex _mutex;

        uint32_t _resolution;
        uint32_t _tileSize;
        uint32_t _tilesPerRow;

        std::unique_ptr<Vk::Image> _atlasImage;
        std::unique_ptr<Vk::Image> _atlasDepthImage;
        std::unique_ptr<Vk::Image> _scratchColorImage;
        std::unique_ptr<Vk::Image> _scratchBloomImage;

        std::queue<uint32_t> _freeTiles;
        std::unordered_map<uint64_t, uint32_t> _resourceToTile;
        std::unordered_map<PreviewResourceType, std::unordered_set<uint32_t>> _dirtyResources;
    };
}