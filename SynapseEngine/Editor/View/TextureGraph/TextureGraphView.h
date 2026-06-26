#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/TextureGraph/TextureGraphViewModel.h"

namespace ax {
    namespace NodeEditor {
        struct EditorContext;
    }
}

namespace Syn {
    class TextureGraphView : public IView<TextureGraphViewModel> {
    public:
        TextureGraphView();
        ~TextureGraphView() override;

        TextureGraphView(const TextureGraphView&) = delete;
        TextureGraphView& operator=(const TextureGraphView&) = delete;
        TextureGraphView(TextureGraphView&& other) noexcept;
        TextureGraphView& operator=(TextureGraphView&& other) noexcept;

        void Draw(TextureGraphViewModel& vm) override;
    private:
        ax::NodeEditor::EditorContext* _context = nullptr;
    };
}