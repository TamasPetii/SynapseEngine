#pragma once
#include <memory>

namespace Syn 
{
    class IGuiWindow {
    public:
        virtual ~IGuiWindow() = default;
        virtual void UpdateAndDraw() = 0;
    };

    template <typename TView, typename TViewModel>
    class EditorWindow : public IGuiWindow {
    public:
        EditorWindow(TView view, TViewModel viewModel)
            : _view(std::move(view)), _viewModel(std::move(viewModel)) {}

        void UpdateAndDraw() override {
            _viewModel.SyncWithEngine();
            _view.Draw(_viewModel);
        }
    private:
        TView _view;
        TViewModel _viewModel;
    };
}