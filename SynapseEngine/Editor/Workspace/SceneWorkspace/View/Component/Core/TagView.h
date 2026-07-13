#pragma once
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/SceneWorkspace/Component/Core/Tag/TagViewModel.h"

namespace Syn {
    class TagView : public IView<TagViewModel> {
    public:
        void Draw(TagViewModel& vm) override;
		void SetActiveEntity(uint32_t entityId) { _entityId = entityId; }
    private:
		uint32_t _entityId = NULL_ENTITY;
        bool _isCardOpen = true;
    };
}