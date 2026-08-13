// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "TransformSetupSystem.h"
#include "Engine/Scene/Scene.h"

namespace Syn
{
    std::vector<TypeID> TransformSetupSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TransformSetupSystem>::ID };
    }

    void TransformSetupSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto hierarchyPool = registry->GetPool<HierarchyComponent>();
        auto hierarchyManager = scene->GetHierarchyManager();

        if (!transformPool || !hierarchyPool || !hierarchyManager) return;

        auto* workQueue = hierarchyManager->EnsureWorkQueue<TransformComponent>();

        auto initTask = this->EmplaceTask(subflow, "InitTransformQueue", [workQueue, hierarchyManager]() {
            workQueue->Initialize(hierarchyManager->GetLevels());
            });

        auto seedTasks = ParallelForEachIf<UPDATE_BIT>(transformPool, subflow, "SeedTransformQueue", [transformPool, hierarchyPool, workQueue](EntityID entity) {

            uint32_t level = 0;
            if (hierarchyPool->Has(entity)) {
                level = hierarchyPool->Get(entity).depthLevel;
            }

            workQueue->Push(level, entity);
            transformPool->SetBit<QUEUED_BIT>(entity);

            });

        for (auto& st : seedTasks) {
            initTask.precede(st);
        }

        uint32_t maxLevel = hierarchyManager->GetMaxActiveLevel();
        std::vector<tf::Task> propagateTasks;

        for (uint32_t level = 0; level < maxLevel; ++level)
        {
            tf::Task propTask = this->EmplaceTask(subflow, "PropagateDirty_" + std::to_string(level), [=](tf::Subflow& nested_subflow) {
                auto currentQueue = workQueue->GetQueue(level);

                if (currentQueue.empty()) return;

                nested_subflow.for_each(currentQueue.begin(), currentQueue.end(), [=](EntityID entity) {

                    if (!hierarchyPool->Has(entity)) return;

                    EntityID child = hierarchyPool->Get(entity).firstChild;

                    while (child != NULL_ENTITY)
                    {
                        if (transformPool->Has(child))
                        {
                            if (!transformPool->IsBitSet<QUEUED_BIT>(child))
                            {
                                transformPool->SetBit<QUEUED_BIT>(child);

                                uint32_t childLevel = hierarchyPool->Get(child).depthLevel;
                                workQueue->Push(childLevel, child);

                                if (transformPool->IsStatic(child))
                                    transformPool->MarkStaticDirty(child);
                                else if (transformPool->IsDynamic(child))
                                    transformPool->SetBit<UPDATE_BIT>(child);
                            }
                        }

                        child = hierarchyPool->Get(child).nextSibling;
                    }
                    });
                });

            if (!propagateTasks.empty()) {
                propagateTasks.back().precede(propTask);
            }
            else {
                for (auto& st : seedTasks) {
                    st.precede(propTask);
                }
                initTask.precede(propTask);
            }

            propagateTasks.push_back(propTask);
        }
    }
}