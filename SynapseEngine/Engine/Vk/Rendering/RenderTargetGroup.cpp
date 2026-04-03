#include "RenderTargetGroup.h"
#include "Engine/Logger/SynLog.h"

namespace Syn::Vk {

    void RenderTargetGroup::AddAttachment(const std::string& name, const ImageConfig& config) {
        _configs[name] = config;

        if (_width == 0 || _height == 0) {
            _width = config.width;
            _height = config.height;
        }

        _images[name] = std::make_unique<Image>(config);
    }

    void RenderTargetGroup::Resize(uint32_t width, uint32_t height) {
        if (_width == width && _height == height) {
            return;
        }

        _width = width;
        _height = height;

        for (auto& [name, config] : _configs) {
            config.width = width;
            config.height = height;
        }

        CreateOrRecreateImages();
    }

    Image* RenderTargetGroup::GetImage(const std::string& name) const {
        auto it = _images.find(name);
        if (it != _images.end()) {
            return it->second.get();
        }

        SYN_ASSERT(false, ("RenderTargetGroup: Image not found - " + name).c_str());
        return nullptr;
    }

    void RenderTargetGroup::CreateOrRecreateImages() {
        _images.clear();

        for (const auto& [name, config] : _configs) {
            _images[name] = std::make_unique<Image>(config);
        }
    }
}