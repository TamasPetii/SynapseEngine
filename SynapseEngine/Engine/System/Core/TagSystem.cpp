#include "TagSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/HierarchyComponent.h"
#include "Engine/System/Core/HierarchySystem.h"

namespace Syn
{
    std::vector<TypeID> TagSystem::GetReadDependencies() const
    {
        return { TypeInfo<HierarchySystem>::ID };
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

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [tagPool, hierarchyPool, hierarchyManager]() {

            uint32_t maxLevel = hierarchyManager->GetMaxActiveLevel();

            for (uint32_t level = 0; level < maxLevel; ++level)
            {
                auto entitiesInLevel = hierarchyManager->GetEntitiesInLevel(level);

                for (EntityID entity : entitiesInLevel)
                {
                    if (!tagPool->Has(entity)) continue;

                    auto& tag = tagPool->Get(entity);
                    bool parentGlobalEnabled = true;

                    if (level > 0)
                    {
                        EntityID parentId = hierarchyPool->Get(entity).parent;
                        if (parentId != NULL_ENTITY && tagPool->Has(parentId))
                        {
                            parentGlobalEnabled = tagPool->Get(parentId).globalEnabled;
                        }
                    }

                    bool newGlobalEnabled = tag.localEnabled && parentGlobalEnabled;

                    if (tag.globalEnabled != newGlobalEnabled || tagPool->IsBitSet<UPDATE_BIT>(entity) || tagPool->IsBitSet<INDEX_CHANGED_BIT>(entity))
                    {
                        tag.globalEnabled = newGlobalEnabled;
                        tagPool->SetBit<CHANGED_BIT>(entity);
                        tag.version++;
                    }
                }
            }
            });
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
                if (tagComp.castShadow)    flags |= (1 << 1);
                if (tagComp.receiveShadow) flags |= (1 << 2);

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