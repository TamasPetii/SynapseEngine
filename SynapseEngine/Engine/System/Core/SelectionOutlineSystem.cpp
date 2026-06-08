#include "SelectionOutlineSystem.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"

namespace Syn
{
    std::vector<TypeID> SelectionOutlineSystem::GetWriteDependencies() const {
        return { TypeInfo<SelectionOutlineSystem>::ID };
    }

    void SelectionOutlineSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto selectedEntity = scene->GetSelectedEntity();
        auto hierarchyManager = scene->GetHierarchyManager();
        uint64_t currentHierarchyVersion = hierarchyManager ? hierarchyManager->GetVersion() : 0;

        if (selectedEntity != _lastSelectedEntity || currentHierarchyVersion != _lastHierarchyVersion)
        {
            _lastSelectedEntity = selectedEntity;
            _lastHierarchyVersion = currentHierarchyVersion;

            auto hierarchyPool = scene->GetRegistry()->GetPool<HierarchyComponent>();
            if (!hierarchyPool) return;

            _selectionMask.assign(hierarchyPool->Size(), 0);

            if (selectedEntity != NULL_ENTITY && hierarchyPool->Has(selectedEntity))
            {
                auto denseIndex = hierarchyPool->GetMapping().Get(selectedEntity);
                if (denseIndex != NULL_INDEX && denseIndex < _selectionMask.size()) {
                    _selectionMask[denseIndex] = 1;
                }

                if (scene->GetSettings()->enableSelectedHierarchyOutline)
                {
                    std::vector<EntityID> queue;
                    auto& comp = hierarchyPool->Get(selectedEntity);
                    if (comp.firstChild != NULL_ENTITY) queue.push_back(comp.firstChild);

                    while (!queue.empty())
                    {
                        EntityID current = queue.back();
                        queue.pop_back();

                        if (hierarchyPool->Has(current))
                        {
                            auto idx = hierarchyPool->GetMapping().Get(current);
                            if (idx != NULL_INDEX && idx < _selectionMask.size()) {
                                _selectionMask[idx] = 1;
                            }

                            auto& currentComp = hierarchyPool->Get(current);
                            if (currentComp.nextSibling != NULL_ENTITY) queue.push_back(currentComp.nextSibling);
                            if (currentComp.firstChild != NULL_ENTITY) queue.push_back(currentComp.firstChild);
                        }
                    }
                }
            }

            uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;
            this->SetFramesToUpload(framesInFlight);
        }
    }

    void SelectionOutlineSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::UploadGPU, [this, scene, frameIndex]() {
            if (!this->ShouldForceUpload()) return;

            auto outlineBufferView = scene->GetComponentBufferManager()->GetComponentBuffer(BufferNames::SelectionOutlineData, frameIndex);
            if (!outlineBufferView.buffer || _selectionMask.empty()) return;

            void* dst = outlineBufferView.buffer->Map();
            std::memcpy(dst, _selectionMask.data(), _selectionMask.size() * sizeof(uint32_t));
            });
    }

    void SelectionOutlineSystem::OnFinish(Scene* scene, tf::Subflow& subflow)
    {
        this->EmplaceTask(subflow, SystemPhaseNames::Finish, [this]() {
            this->DecrementFramesToUpload();
            });
    }
}