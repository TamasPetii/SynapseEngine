#include "PipelineOverrideViewModel.h"

namespace Syn
{
    PipelineOverrideViewModel::PipelineOverrideViewModel(ISelectionApi* selectionApi, IPipelineOverrideApi* overrideApi)
        : _selectionApi(selectionApi), _overrideApi(overrideApi) {}

    const PipelineOverrideState& PipelineOverrideViewModel::GetState() const { return _state; }

    void PipelineOverrideViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_overrideApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _overrideApi->HasPipelineOverride(activeEntity))
        {
            _state.hasComponent = true;

            _state.expectedSlotCount = _overrideApi->GetExpectedSlotCount(activeEntity);
            _state.sharedPipelineEntity = _overrideApi->GetSharedPipelineEntity(activeEntity);

            _state.availablePipelines = _overrideApi->GetAvailablePipelines();
            //_state.compatibleSharedEntities = _overrideApi->GetCompatibleSharedEntities(activeEntity);

            _state.overrides.clear();
            _state.overrides.reserve(_state.expectedSlotCount);
            for (uint32_t i = 0; i < _state.expectedSlotCount; ++i) {
                _state.overrides.push_back(_overrideApi->GetPipelineAtSlot(activeEntity, i));
            }
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void PipelineOverrideViewModel::Dispatch(const PipelineOverrideIntent& intent)
    {
        EntityID active = _selectionApi->GetSelectedEntity();
        if (active == NULL_ENTITY)
            return;

        std::visit([this, active](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetPipelineOverrideSlotIntent>) {
                    if (arg.slotIndex < _state.overrides.size()) {
                        _state.overrides[arg.slotIndex] = arg.pipelineType;
                        _overrideApi->SetPipelineAtSlot(active, arg.slotIndex, arg.pipelineType);
                    }
                }
                else if constexpr (std::is_same_v<T, SetSharedPipelineEntityIntent>) {
                    _state.sharedPipelineEntity = arg.sharedEntity;
                    _overrideApi->SetSharedPipelineEntity(active, arg.sharedEntity);
                }
            }, intent);
    }
}