#include "RenderTargetManager.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    RenderTargetManager::RenderTargetManager(uint32_t framesInFlight)
        : _framesInFlight(framesInFlight) {}

    void RenderTargetManager::CreateGroup(const std::string& name) {
        if (_groups.find(name) != _groups.end()) {
            SYN_ASSERT(false, ("RenderTargetManager: Group already exists - " + name).c_str());
            return;
        }

        _groups[name].resize(_framesInFlight);
        for (uint32_t i = 0; i < _framesInFlight; ++i) {
            _groups[name][i] = std::make_unique<Vk::RenderTargetGroup>();
        }
    }

    void RenderTargetManager::AddAttachment(const std::string& groupName, const std::string& attachmentName, const Vk::ImageConfig& config) {
        auto it = _groups.find(groupName);
        if (it != _groups.end()) {
            for (uint32_t i = 0; i < _framesInFlight; ++i) {
                it->second[i]->AddAttachment(attachmentName, config);
            }
        }
        else {
            SYN_ASSERT(false, ("RenderTargetManager: Cannot add attachment, Group not found - " + groupName).c_str());
        }
    }

    Vk::RenderTargetGroup* RenderTargetManager::GetGroup(const std::string& name, uint32_t frameIndex) const {
        auto it = _groups.find(name);
        if (it != _groups.end()) {
            return it->second[frameIndex].get();
        }

        //SYN_ASSERT(false, ("RenderTargetManager: Group not found - " + name).c_str());
        return nullptr;
    }

    void RenderTargetManager::Resize(uint32_t frameIndex, uint32_t width, uint32_t height) {
        for (auto& [name, groupVector] : _groups) {
            groupVector[frameIndex]->Resize(width, height);
        }
    }

    void RenderTargetManager::RemoveGroup(const std::string& name) {
        _groups.erase(name);
    }
}