#pragma once

namespace Syn {
    template <typename TViewModel>
    class IView {
    public:
        virtual ~IView() = default;
        virtual void Draw(TViewModel& viewModel) = 0;
    };
}