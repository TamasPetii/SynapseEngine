#pragma once
#include "Engine/Scene/SceneManager.h"
#include "Engine/Scene/Scene.h"
#include "EditorCore/Types/EntityHandle.h"
#include <type_traits>
#include <concepts>

namespace Syn {

    template <typename T>
    concept HasStorageCategory = requires(T pool, EntityID entity) {
        pool->SetCategory(entity, pool->GetCategory(entity));
    };

    class EditorApiUtils {
    public:
        template <typename T, typename Getter, typename ReturnType>
        static ReturnType ReadComponent(SceneManager* sm, EntityID entity, Getter&& getter, ReturnType defaultValue) {
            auto scene = sm->GetActiveScene();
            if (!scene) return defaultValue;

            auto registry = scene->GetRegistry();
            if (!registry || !registry->HasComponent<T>(entity)) return defaultValue;

            return getter(registry->GetComponent<T>(entity));
        }

        template <typename T, typename Modifier>
        static void ModifyComponent(SceneManager* sm, EntityID entity, Modifier&& modifier) {
            auto scene = sm->GetActiveScene();
            if (!scene) return;

            auto registry = scene->GetRegistry();
            if (!registry || !registry->HasComponent<T>(entity)) return;

            auto pool = registry->GetPool<T>();

            modifier(pool->Get(entity), pool);

            if (pool->IsStatic(entity)) {
                pool->MarkStaticDirty(entity);
            }
            else if (pool->IsDynamic(entity)) {
                pool->template SetBit<UPDATE_BIT>(entity);
            }
        }

        template <typename T>
        static bool HasComponent(SceneManager* sm, EntityID entity) {
            auto scene = sm->GetActiveScene();
            if (!scene) return false;
            auto registry = scene->GetRegistry();
            return registry && registry->HasComponent<T>(entity);
        }


        template <typename... Components>
        static void CloneEntityComponents(SceneManager* sm, EntityID src, EntityID dst) {
            auto scene = sm->GetActiveScene();
            if (!scene) return;

            auto registry = scene->GetRegistry();
            if (!registry) return;

            (
                [&]() {
                    auto pool = registry->GetPool<Components>();
                    if (pool && pool->Has(src)) {

                        if constexpr (std::is_empty_v<Components>) {
                            pool->Add(dst);
                        }
                        else {
                            registry->AddComponent<Components>(dst);
                            registry->GetComponent<Components>(dst) = registry->GetComponent<Components>(src);
                        }

                        if constexpr (HasStorageCategory<decltype(pool)>) {
                            pool->SetCategory(dst, pool->GetCategory(src));
                        }
                    }
                }(), ... 
                    );
        }
    };
}