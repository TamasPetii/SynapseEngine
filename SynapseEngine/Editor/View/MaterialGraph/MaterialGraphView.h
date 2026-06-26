#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/MaterialGraph/MaterialGraphViewModel.h"

namespace ax {
    namespace NodeEditor {
        struct EditorContext;
    }
}

namespace Syn {
    class MaterialGraphView : public IView<MaterialGraphViewModel> {
    public:
        MaterialGraphView();
        ~MaterialGraphView() override;

        MaterialGraphView(const MaterialGraphView&) = delete;
        MaterialGraphView& operator=(const MaterialGraphView&) = delete;

        MaterialGraphView(MaterialGraphView&& other) noexcept;
        MaterialGraphView& operator=(MaterialGraphView&& other) noexcept;

        void Draw(MaterialGraphViewModel& vm) override;
    private:
        const char* GetPinName(GraphPinType type);
        void DrawMaterialNode(const GraphNodeData& node);
        void DrawTextureNode(const GraphNodeData& node);
    private:
        ax::NodeEditor::EditorContext* _context = nullptr;
    };
}