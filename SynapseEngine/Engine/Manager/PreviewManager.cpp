#include "PreviewManager.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    PreviewManager::PreviewManager(uint32_t initialResolution, uint32_t tileSize)
        : _resolution(initialResolution), _tileSize(tileSize), _tilesPerRow(0)
    {
        CreateOrResizeAtlas(_resolution);
    }

    uint64_t PreviewManager::GetUniqueId(PreviewResourceType type, uint32_t resourceId) const {
        return (static_cast<uint64_t>(type) << 32) | resourceId;
    }

    void PreviewManager::CreateOrResizeAtlas(uint32_t newResolution) {
        uint32_t oldTotalTiles = _tilesPerRow * _tilesPerRow;

        _resolution = newResolution;
        _tilesPerRow = _resolution / _tileSize;
        uint32_t newTotalTiles = _tilesPerRow * _tilesPerRow;

        Vk::ImageConfig config{};
        config.width = _resolution;
        config.height = _resolution;
        config.type = VK_IMAGE_TYPE_2D;
        config.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        config.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        config.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        config.AddView(Vk::ImageViewNames::Default, { .viewType = VK_IMAGE_VIEW_TYPE_2D });

        _atlasImage = std::make_unique<Vk::Image>(config);

        for (uint32_t i = oldTotalTiles; i < newTotalTiles; ++i) {
            _freeTiles.push(i);
        }

        for (const auto& [id, tileIndex] : _resourceToTile) {
            PreviewResourceType type = static_cast<PreviewResourceType>(id >> 32);
            uint32_t resourceId = static_cast<uint32_t>(id & 0xFFFFFFFF);
            _dirtyResources[type].insert(resourceId);
        }

        Info("PreviewManager: Atlas resized to {}x{}", _resolution, _resolution);
    }

    bool PreviewManager::AllocateTile(PreviewResourceType type, uint32_t resourceId) {
        std::lock_guard<std::mutex> lock(_mutex);
        uint64_t id = GetUniqueId(type, resourceId);

        if (_resourceToTile.find(id) != _resourceToTile.end()) return true;

        if (_freeTiles.empty()) {
            CreateOrResizeAtlas(_resolution * 2);
        }

        uint32_t tileIndex = _freeTiles.front();
        _freeTiles.pop();

        _resourceToTile[id] = tileIndex;
        return true;
    }

    void PreviewManager::FreeTile(PreviewResourceType type, uint32_t resourceId) {
        std::lock_guard<std::mutex> lock(_mutex);
        uint64_t id = GetUniqueId(type, resourceId);
        auto it = _resourceToTile.find(id);

        if (it != _resourceToTile.end()) {
            _freeTiles.push(it->second);
            _resourceToTile.erase(it);
            _dirtyResources[type].erase(resourceId);
        }
    }

    bool PreviewManager::HasTile(PreviewResourceType type, uint32_t resourceId) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _resourceToTile.find(GetUniqueId(type, resourceId)) != _resourceToTile.end();
    }

    void PreviewManager::MarkDirty(PreviewResourceType type, uint32_t resourceId) {
        std::lock_guard<std::mutex> lock(_mutex);
        _dirtyResources[type].insert(resourceId);
    }

    std::vector<uint32_t> PreviewManager::ConsumeDirtyResources(PreviewResourceType type) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto& dirtySet = _dirtyResources[type];

        std::vector<uint32_t> result(dirtySet.begin(), dirtySet.end());
        dirtySet.clear();

        return result;
    }

    void PreviewManager::GetViewportAndScissor(PreviewResourceType type, uint32_t resourceId, VkViewport& outViewport, VkRect2D& outScissor) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _resourceToTile.find(GetUniqueId(type, resourceId));
        if (it == _resourceToTile.end()) return;

        uint32_t tileIndex = it->second;
        uint32_t col = tileIndex % _tilesPerRow;
        uint32_t row = tileIndex / _tilesPerRow;

        float x = static_cast<float>(col * _tileSize);
        float y = static_cast<float>(row * _tileSize);

        outViewport.x = x;
        outViewport.y = y;
        outViewport.width = static_cast<float>(_tileSize);
        outViewport.height = static_cast<float>(_tileSize);
        outViewport.minDepth = 0.0f;
        outViewport.maxDepth = 1.0f;

        outScissor.offset = { static_cast<int32_t>(x), static_cast<int32_t>(y) };
        outScissor.extent = { _tileSize, _tileSize };
    }

    void PreviewManager::GetNormalizedUVs(PreviewResourceType type, uint32_t resourceId, glm::vec2& outUv0, glm::vec2& outUv1) const {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _resourceToTile.find(GetUniqueId(type, resourceId));
        if (it == _resourceToTile.end()) {
            outUv0 = glm::vec2(0.0f); outUv1 = glm::vec2(1.0f);
            return;
        }

        uint32_t tileIndex = it->second;
        uint32_t col = tileIndex % _tilesPerRow;
        uint32_t row = tileIndex / _tilesPerRow;

        float normalizedTileSize = 1.0f / static_cast<float>(_tilesPerRow);

        outUv0 = glm::vec2(col * normalizedTileSize, row * normalizedTileSize);
        outUv1 = glm::vec2((col + 1) * normalizedTileSize, (row + 1) * normalizedTileSize);
    }

    std::vector<uint32_t> PreviewManager::GetActiveResources(PreviewResourceType type) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<uint32_t> result;

        uint64_t typePrefix = static_cast<uint64_t>(type) << 32;
        uint64_t typeMask = 0xFFFFFFFF00000000;

        for (const auto& [id, tileIndex] : _resourceToTile) {
            if ((id & typeMask) == typePrefix) {
                result.push_back(static_cast<uint32_t>(id & 0xFFFFFFFF));
            }
        }

        return result;
    }
}