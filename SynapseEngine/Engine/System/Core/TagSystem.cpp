#include "TagSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include "TagSetupSystem.h"

namespace Syn
{
    std::vector<TypeID> TagSystem::GetReadDependencies() const
    {
        return { TypeInfo<TagSetupSystem>::ID };
    }

    std::vector<TypeID> TagSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TagSystem>::ID };
    }

    void TagSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto tagPool = registry->GetPool<TagComponent>();
        auto hierarchyPool = registry->GetPool<HierarchyComponent>();
        auto hierarchyManager = scene->GetHierarchyManager();

        if (!tagPool || !hierarchyPool || !hierarchyManager) return;

        auto* workQueue = hierarchyManager->EnsureWorkQueue<TagComponent>();
        uint32_t maxLevel = hierarchyManager->GetMaxActiveLevel();
        std::vector<tf::Task> levelTasks;

        for (uint32_t level = 0; level < maxLevel; ++level)
        {
            tf::Task levelTask = this->EmplaceTask(subflow, "TagEnable_" + std::to_string(level), [=](tf::Subflow& nested_subflow) {

                auto currentQueue = workQueue->GetQueue(level);

                if (currentQueue.empty()) return;

                nested_subflow.for_each(currentQueue.begin(), currentQueue.end(), [=](EntityID entity) {

                    auto& tag = tagPool->Get(entity);

                    bool parentGlobalEnabled = true;
                    if (hierarchyPool->Has(entity))
                    {
                        EntityID parent = hierarchyPool->Get(entity).parent;

                        if (parent != NULL_ENTITY && tagPool->Has(parent)) {
                            parentGlobalEnabled = tagPool->Get(parent).globalEnabled;
                        }
                    }

                    bool newGlobalEnabled = tag.localEnabled && parentGlobalEnabled;
                    if (tag.globalEnabled != newGlobalEnabled)
                    {
                        tag.globalEnabled = newGlobalEnabled;

                        if (tagPool->IsDynamic(entity)) {
                            tagPool->SetBit<CHANGED_BIT>(entity);
                        }

                        tag.version++;
                    }
                    });
                });

            if (!levelTasks.empty()) {
                levelTasks.back().precede(levelTask);
            }

            levelTasks.push_back(levelTask);
        }
    }

    void TagSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto tagPool = registry->GetPool<TagComponent>();
        if (!tagPool) return;

        auto tagDataBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TagData, frameIndex);
        if (!tagDataBuffer.buffer) return;

        auto tagDataBufferHandler = static_cast<uint32_t*>(tagDataBuffer.buffer->Map());
        bool forceUpload = this->ShouldForceUpload();

        auto processUpload = [tagPool, tagDataBuffer, tagDataBufferHandler, forceUpload](EntityID entity) {
            auto& tagComp = tagPool->Get(entity);
            auto tagIndex = tagPool->GetMapping().Get(entity);

            if (forceUpload || tagDataBuffer.versions[tagIndex] != tagComp.version)
            {
                tagDataBuffer.versions[tagIndex] = tagComp.version;

                uint32_t flags = 0;
                if (tagComp.globalEnabled) flags |= (1 << 0);

                tagDataBufferHandler[tagIndex] = flags;
            }
            };

        ForEachStream(tagPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
            ForEachDynamic(tagPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(tagPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}