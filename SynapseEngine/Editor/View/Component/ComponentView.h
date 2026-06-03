#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/ComponentViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class ComponentView : public IView<ComponentViewModel> {
    public:
        void Draw(ComponentViewModel& vm) override;
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}