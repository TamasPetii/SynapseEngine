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

#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"

#include "Engine/Registry/Registry.h"
#include "Engine/Registry/Pool/PoolTypes.h"
#include "Engine/Registry/Insiders/RegistryInsider.h"
#include "Engine/Serialization/Archive/Output/IOutputArchive.h"

#include "PoolSchema.h"
#include "FlatStorageImplSchema.h"
#include "SegmentedStorageImplSchema.h"
#include "Engine/Serialization/Schema/Component/ComponentSchemas.h"

#include <type_traits>

namespace Syn
{
    template <typename... Components>
    struct RegistrySnapshot
    {
        Registry& registry;
    };
    
    template <typename T>
    concept SegmentedPoolConstraint = requires(T pool, EntityID entity, StorageCategory cat) {
        pool->SetCategory(entity, cat);
    };

    template <typename Archive, typename Component>
    void SerializeEntityComponent(Archive& ar, Registry& reg, EntityID entity, const char* compName)
    {
        if constexpr (std::is_base_of_v<IOutputArchive, Archive>)
        {
            auto pool = reg.GetPool<Component>();

            if (pool->Has(entity))
            {
                if constexpr (std::is_empty_v<Component>) {
                    ScopedArchiveObject tagObj(ar, compName);
                }
                else {
                    auto& comp = reg.GetComponent<Component>(entity);
                    ar.Property(compName, const_cast<std::remove_const_t<Component>&>(comp));

                    if constexpr (SegmentedPoolConstraint<decltype(pool)>)
                    {
                        auto denseIdx = pool->GetMapping().Get(entity);

                        uint32_t catEnum = 2;
                        if (pool->GetStorage().IsStatic(denseIdx))
                            catEnum = 0;
                        else if (pool->GetStorage().IsDynamic(denseIdx)) 
                            catEnum = 1;

                        std::string catKey = std::string(compName) + "_Category";
                        ar.Property(catKey.c_str(), catEnum);
                    }
                }
            }
        }
        else
        {
            reg.EnsurePool<Component>();
            auto pool = reg.GetPool<Component>();

            if constexpr (std::is_empty_v<Component>) 
            {
                if (ar.HasProperty(compName))
                {
                    pool->Add(entity);
                }
            }
            else
            {
                if (ar.HasProperty(compName))
                {
                    Component comp{};
                    ar.Property(compName, comp);

                    pool->Add(entity, std::move(comp));

                    if constexpr (SegmentedPoolConstraint<decltype(pool)>)
                    {
                        uint32_t catEnum = 2;
                        std::string catKey = std::string(compName) + "_Category";
                        
                        if (ar.HasProperty(catKey.c_str())) {
                            ar.Property(catKey.c_str(), catEnum);
                        }
                        
                        StorageCategory targetCategory = StorageCategory::Stream;
                        if (catEnum == 0)
                            targetCategory = StorageCategory::Static;
                        else if (catEnum == 1)
                            targetCategory = StorageCategory::Dynamic;

                        pool->SetCategory(entity, targetCategory);
                    }
                }
            }
        }
    }

    template <typename... Components>
    struct Schema<RegistrySnapshot<Components...>>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);

            auto& snapshot = const_cast<std::remove_const_t<U>&>(val);
            Registry& reg = snapshot.registry;

            auto& entityCounter = RegistryInsider::GetEntityCounter(reg, RegistryInsider::GetKey());
            auto& freeEntities = RegistryInsider::GetFreeEntities(reg, RegistryInsider::GetKey());
            auto& activeEntities = RegistryInsider::GetActiveEntities(reg, RegistryInsider::GetKey());

            constexpr bool isInput = std::is_base_of_v<IInputArchive, Archive>;

            if (ar.IsBinary()) 
            {
                if constexpr (isInput)
                    reg.Clear();

                ar.Property("entityCounter", entityCounter);

                BlitVector<EntityID> blitFree{ freeEntities };
                ar.Property("freeEntities", blitFree);

                //SparseSet is a pool, it has a schema
                ar.Property("activeEntities", activeEntities);
    
                (ar.Property(ComponentName<Components>::value, reg.EnsurePool<Components>()->_pool), ...);
            }
            else
            {
                if constexpr (isInput)
                    reg.Clear();

                ar.Property("entityCounter", entityCounter);
                ar.Property("freeEntities", freeEntities);

                uint32_t entCount = 0;
                if constexpr (!isInput)
                    entCount = static_cast<uint32_t>(activeEntities.GetDenseEntities().size());

                ar.EnterArray("Entities", entCount);

                for (uint32_t i = 0; i < entCount; ++i)
                {
                    ar.EnterObject("Entity");

                    EntityID id = 0;
                    if constexpr (!isInput)
                        id = activeEntities.GetDenseEntities()[i];

                    ar.Property("ID", id);

                    if constexpr (isInput)
                        activeEntities.Add(id);

                    (SerializeEntityComponent<Archive, Components>(ar, reg, id, ComponentName<Components>::value), ...);

                    ar.LeaveObject();
                }
                ar.LeaveArray();
            }
        }
    };
}