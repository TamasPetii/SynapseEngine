#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Logger/LoggerViewModel.h"
#include <unordered_map>
#include <string>
#include <imgui.h>

namespace Syn {
    class LoggerView : public IView<LoggerViewModel> {
    public:
        void Draw(LoggerViewModel& vm) override;
    private:
        void RenderTopBar(LoggerViewModel& vm, const LoggerState& state);
        void RenderLogTable(const LoggerState& state, float tableHeight);
        ImVec4 GetColorForLevel(LogLevel level) const;
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}