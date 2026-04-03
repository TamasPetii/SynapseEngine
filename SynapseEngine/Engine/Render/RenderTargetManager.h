#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderTargetGroup.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace Syn {

    class SYN_API RenderTargetManager {
    public:
        explicit RenderTargetManager(uint32_t framesInFlight);
        ~RenderTargetManager() = default;

        RenderTargetManager(const RenderTargetManager&) = delete;
        RenderTargetManager& operator=(const RenderTargetManager&) = delete;

        void CreateGroup(const std::string& name);
        void AddAttachment(const std::string& groupName, const std::string& attachmentName, const Vk::ImageConfig& config);

        Vk::RenderTargetGroup* GetGroup(const std::string& name, uint32_t frameIndex) const;

        void Resize(uint32_t frameIndex, uint32_t width, uint32_t height);

        void RemoveGroup(const std::string& name);
    private:
        uint32_t _framesInFlight;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Vk::RenderTargetGroup>>> _groups;
    };
}