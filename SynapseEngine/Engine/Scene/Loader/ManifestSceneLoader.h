#pragma once
#include "ISceneLoader.h"
#include "Engine/Serialization/Schema/Scene/SceneSchema.h"
#include "Engine/Serialization/Schema/Scene/SceneSnapshotTypes.h"
#include <vector>
#include <string>

namespace Syn
{
    struct SYN_API WakeUpHelper
    {
        Registry* registry;

        template<typename T, uint32_t... ExtraBits, typename Func>
        void Run(Func&& customLogic)
        {
            auto pool = registry->GetPool<T>();
            if (!pool) return;

            for (auto entity : pool->GetStorage().GetDenseEntities())
            {
                pool->template SetBit<UPDATE_BIT>(entity);

                if constexpr (sizeof...(ExtraBits) > 0) {
                    (pool->template SetBit<ExtraBits>(entity), ...);
                }

                customLogic(pool->Get(entity), entity);

                if constexpr (requires { pool->GetStorage().IsStatic(0); })
                {
                    auto denseIdx = pool->GetMapping().Get(entity);
                    if (pool->GetStorage().IsStatic(denseIdx))
                        pool->MarkStaticDirty(entity);
                }
            }
        }

        template<typename T, uint32_t... ExtraBits>
        void Run()
        {
            Run<T, ExtraBits...>([](auto&, EntityID) {});
        }
    };

    class SYN_API ManifestSceneLoader : public ISceneLoader
    {
    public:
        ManifestSceneLoader() = default;
        ~ManifestSceneLoader() override = default;

        bool LoadScene(Scene& scene, const std::filesystem::path& path) override;

    private:
        bool DeserializeSnapshot(const std::filesystem::path& path, FullSceneSnapshot& outSnapshot);

        // Model & Animation loading and mapping
        void LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels);
        void LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims);

        // Texture & Material loading and mapping
        void LoadAndMapTextures(const std::vector<TextureManifestEntry>& texManifest, std::vector<uint32_t>& outLocalToGlobalTex);
        void RemapAndLoadMaterials(std::vector<MaterialManifestEntry>& matManifest, const std::vector<uint32_t>& localToGlobalTex, std::vector<uint32_t>& outLocalToGlobalMats);

        // Component remapping
        void RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels);
        void RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims);
        void RemapMaterialComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalMats);

        void WakeUpEntities(Scene& scene);
    };
}