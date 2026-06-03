#pragma once
#include <imgui.h>
#include <glm/glm.hpp>
#include <string>

namespace Syn::UI {
    bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, bool& outDeactivated, float columnWidth = 100.0f);
}