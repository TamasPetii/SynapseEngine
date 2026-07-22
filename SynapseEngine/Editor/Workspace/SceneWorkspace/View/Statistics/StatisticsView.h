#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Statistics/StatisticsViewModel.h"
#include <string>
#include <unordered_map>

namespace Syn {
    class StatisticsView : public IView<StatisticsViewModel> {
    public:
        void Draw(StatisticsViewModel& vm) override;
    private:
        void RenderOverviewCard(const StatisticsState& state);
        void RenderPassStatsCard(StatisticsViewModel& vm, const StatisticsState& state, float mainContentBottomY);
        void RenderGroupRow(const UiStatGroup& group);

        std::unordered_map<std::string, bool> _cardStates;
    };
}