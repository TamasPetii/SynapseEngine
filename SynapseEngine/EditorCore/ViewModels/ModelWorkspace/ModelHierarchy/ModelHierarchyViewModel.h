#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ModelHierarchyState.h"
#include "ModelHierarchyIntent.h"
#include "EditorCore/Api/IModelApi.h"
#include "EditorCore/Api/IPreviewApi.h"
#include <unordered_set>
#include <unordered_map>

namespace Syn {
    class ModelHierarchyViewModel : public IViewModel<ModelHierarchyState, ModelHierarchyIntent> {
    public:
        ModelHierarchyViewModel(IModelApi* modelApi, IPreviewApi* previewApi);
        ~ModelHierarchyViewModel() override = default;

        const ModelHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const ModelHierarchyIntent& intent) override;

    private:
        void RebuildModelList();
        void RebuildFlatList();
        bool TraverseAndFlatten(uint32_t modelId, int32_t descriptorIndex, uint16_t currentTransformIndex, int depth, const CpuModelData& cpuData, const std::unordered_map<uint16_t, std::vector<uint32_t>>& childMap, const std::string& nodeName);
        uint64_t GetNodeKey(uint32_t modelId, int32_t descriptorIndex) const;
    private:
        IModelApi* _modelApi = nullptr;
        IPreviewApi* _previewApi = nullptr;
        ModelHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
        std::unordered_set<uint64_t> _expandedNodes;
    };
}