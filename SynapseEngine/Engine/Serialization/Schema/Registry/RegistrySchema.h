#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Schema/Core/VectorSchema.h"
#include <type_traits>

#include "Engine/Registry/Registry.h"
#include "Engine/Registry/Pool/PoolTypes.h"
#include "Engine/Registry/Insiders/RegistryInsider.h"

#include "PoolSchema.h"
#include "FlatStorageImplSchema.h"
#include "SegmentedStorageImplSchema.h"

namespace Syn
{
    template <typename... Components>
    struct SYN_API RegistrySnapshot
    {
        Registry& registry;
    };

    template <typename Archive, typename Component>
    void SerializeEntityComponent(Archive& ar, Registry& reg, EntityID entity, const char* compName)
    {
        if constexpr (std::is_base_of_v<IOutputArchive, Archive>)
        {
            if (reg.HasComponent<Component>(entity))
            {
                if constexpr (std::is_empty_v<Component>) {
                    ScopedArchiveObject tagObj(ar, compName);
                }
                else {
                    auto& comp = reg.GetComponent<Component>(entity);
                    ar.Property(compName, const_cast<std::remove_const_t<Component>&>(comp));
                }
            }
        }
        else
        {
            if (ar.HasProperty(compName))
            {
                if constexpr (std::is_empty_v<Component>) {
                    reg.AddComponent<Component>(entity);
                }
                else {
                    Component comp;
                    ar.Property(compName, comp);
                    reg.AddComponent<Component>(entity, std::move(comp));
                }
            }
        }
    }

    template <typename... Components>
    struct SYN_API Schema<RegistrySnapshot<Components...>>
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