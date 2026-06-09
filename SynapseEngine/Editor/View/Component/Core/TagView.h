#pragma once
#include "Editor/View/IView.h"
#include "EditorCore/ViewModels/Component/Core/Tag/TagViewModel.h"

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