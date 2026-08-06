#include "AnimationHierarchyViewModel.h"
#include "Editor/Manager/EditorIcons.h"
#include <algorithm>
#include <cctype>

namespace Syn 
{
    AnimationHierarchyViewModel::AnimationHierarchyViewModel(IAnimationApi* animationApi, IModelApi* modelApi, IPreviewApi* previewApi)
        : _animationApi(animationApi), _modelApi(modelApi), _previewApi(previewApi)
    {}

    uint64_t AnimationHierarchyViewModel::GetNodeKey(uint32_t animationId, int32_t descriptorIndex) const {
        return (static_cast<uint64_t>(animationId) << 32) | static_cast<uint32_t>(descriptorIndex);
    }

    void AnimationHierarchyViewModel::SyncWithEngine() {
        if (!_animationApi) return;

        uint64_t currentVersion = _animationApi->GetVersion();
        uint32_t currentSelection = _animationApi->GetSelected();

        if (currentVersion != _lastEngineVersion || _isDirty) {
            RebuildAnimationList();
            RebuildFlatList();
            _lastEngineVersion = currentVersion;
            _isDirty = false;
        }

        if (_state.selectedAnimationId != currentSelection) {
            _state.selectedAnimationId = currentSelection;
            _isDirty = true;
        }

        if (_previewApi) {
            _state.atlasHandle = _previewApi->GetAtlasHandle();

            for (auto& node : _state.filteredAnimations) {
                if (!node.hasPreview) {
                    if (_previewApi->GetPreviewUVs(PreviewResourceType::Animation, node.id, node.uv0, node.uv1)) {
                        node.hasPreview = true;
                    }
                }
            }
        }
    }

    void AnimationHierarchyViewModel::Dispatch(const AnimationHierarchyIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, AnimationHierarchySelectIntent>) {
                if (_animationApi) {
                    _animationApi->SetSelected(arg.animationId);
                    _animationApi->ApplyAnimationToPreviewObject(arg.animationId);
                }

                if (_state.selectedAnimationId != arg.animationId) {
                    _state.selectedAnimationId = arg.animationId;
                    _state.selectedDescriptorIndex = arg.descriptorIndex;
                    _isDirty = true;
                }
                else {
                    _state.selectedDescriptorIndex = arg.descriptorIndex;
                }
            }
            else if constexpr (std::is_same_v<T, AnimationHierarchyToggleExpandIntent>) {
                uint64_t key = GetNodeKey(arg.animationId, arg.descriptorIndex);
                if (arg.expand) _expandedNodes.insert(key);
                else _expandedNodes.erase(key);
                _isDirty = true;
            }
            else if constexpr (std::is_same_v<T, AnimationHierarchySetSearchIntent>) {
                if (_state.searchQuery != arg.query) {
                    _state.searchQuery = arg.query;
                    _isDirty = true;
                }
            }
            else if constexpr (std::is_same_v<T, AnimationHierarchyRefreshIntent>) {
                _isDirty = true;
            }
            }, intent);
    }

    void AnimationHierarchyViewModel::RebuildAnimationList() {
        if (!_animationApi) return;

        _state.filteredAnimations.clear();
        auto allAnims = _animationApi->GetAllAnimations();

        std::string searchLower = _state.searchQuery;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto& anim : allAnims) {
            std::string nameLower = anim.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (searchLower.empty() || nameLower.find(searchLower) != std::string::npos) {
                AnimationItem node;
                node.id = anim.id;
                node.name = anim.name;
                node.path = anim.path;
                node.icon = SYN_ICON_ANIMATION;

                if (_previewApi) {
                    if (_previewApi->GetPreviewUVs(PreviewResourceType::Animation, anim.id, node.uv0, node.uv1)) {
                        node.hasPreview = true;
                    }
                    else {
                        node.hasPreview = false;
                        _previewApi->RequestPreview(PreviewResourceType::Animation, anim.id);
                    }
                }

                _state.filteredAnimations.push_back(node);
            }
        }
    }

    void AnimationHierarchyViewModel::RebuildFlatList() {
        _state.flatNodes.clear();
        if (!_animationApi || !_modelApi || _state.selectedAnimationId == 0xFFFFFFFF) return;

        const CpuAnimationData* animData = _animationApi->GetAnimationCpuData(_state.selectedAnimationId);
        if (!animData) return;

        uint32_t baseModelId = animData->baseModelId;
        const CpuModelData* cpuData = _modelApi->GetModelCpuData(baseModelId);
        if (!cpuData) return;

        std::unordered_map<uint16_t, std::vector<uint32_t>> childMap;
        for (uint32_t i = 0; i < cpuData->meshNodeDescriptors.size(); ++i) {
            childMap[cpuData->meshNodeDescriptors[i].parentNodeIndex].push_back(i);
        }

        std::string nodeName = "Root";
        auto allAnims = _animationApi->GetAllAnimations();
        for (const auto& a : allAnims) {
            if (a.id == _state.selectedAnimationId) {
                nodeName = a.name;
                break;
            }
        }

        TraverseAndFlatten(_state.selectedAnimationId, -1, 0xFFFF, 0, *cpuData, childMap, nodeName, *animData);
    }

    bool AnimationHierarchyViewModel::TraverseAndFlatten(uint32_t animationId, int32_t descriptorIndex, uint16_t currentTransformIndex, int depth, const CpuModelData& cpuData, const std::unordered_map<uint16_t, std::vector<uint32_t>>& childMap, const std::string& nodeName, const CpuAnimationData& animData) {

        bool matchesSearch = _state.searchQuery.empty() ||
            std::search(nodeName.begin(), nodeName.end(), _state.searchQuery.begin(), _state.searchQuery.end(),
                [](char c1, char c2) { return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2)); }) != nodeName.end();

        bool hasChildren = childMap.contains(currentTransformIndex);

        uint64_t key = GetNodeKey(animationId, descriptorIndex);
        bool isExpanded = _expandedNodes.contains(key) || !_state.searchQuery.empty();

        size_t flattenIndex = _state.flatNodes.size();

        AnimationHierarchyNode node;
        node.animationId = animationId;
        node.descriptorIndex = descriptorIndex;
        node.name = nodeName;
        node.depth = depth;
        node.hasChildren = hasChildren;
        node.isExpanded = isExpanded;

        if (descriptorIndex == -1) {
            node.icon = SYN_ICON_FILM;
        }
        else {
            node.icon = SYN_ICON_PROJECT_DIAGRAM;
        }

        _state.flatNodes.push_back(node);

        bool anyChildMatches = false;

        if (isExpanded && hasChildren) {
            for (uint32_t childDescIdx : childMap.at(currentTransformIndex)) {
                const auto& childDesc = cpuData.meshNodeDescriptors[childDescIdx];
                std::string childName = childDesc.name;

                if (childName.empty())
                    childName = "Node_" + std::to_string(childDescIdx);

                if (TraverseAndFlatten(animationId, childDescIdx, childDesc.nodeIndex, depth + 1, cpuData, childMap, childName, animData)) {
                    anyChildMatches = true;
                }
            }
        }

        if (!_state.searchQuery.empty() && !matchesSearch && !anyChildMatches) {
            _state.flatNodes.resize(flattenIndex);
            return false;
        }

        return true;
    }
}