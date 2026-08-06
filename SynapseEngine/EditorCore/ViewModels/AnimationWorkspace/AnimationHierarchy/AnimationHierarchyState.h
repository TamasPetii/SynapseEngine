#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include "EditorCore/Types/TextureHandle.h"

namespace Syn
{
    struct AnimationItem {
        uint32_t id;
        std::string name;
        std::string path;
        std::string icon;

        glm::vec2 uv0{ 0.0f, 0.0f };
        glm::vec2 uv1{ 1.0f, 1.0f };
        bool hasPreview = false;
    };

    struct AnimationHierarchyNode {
        uint32_t animationId = 0xFFFFFFFF;
        int32_t descriptorIndex = -1;

        std::string name;
        std::string icon;

        int depth = 0;
        bool isExpanded = true;
        bool hasChildren = false;

        bool isAnimatedNode = false;
    };

    struct AnimationHierarchyState {
        std::vector<AnimationItem> filteredAnimations;
        std::vector<AnimationHierarchyNode> flatNodes;

        uint32_t selectedAnimationId = 0xFFFFFFFF;
        int32_t selectedDescriptorIndex = -1;
        std::string searchQuery = "";

        TextureHandle atlasHandle = InvalidTextureHandle;
    };
}