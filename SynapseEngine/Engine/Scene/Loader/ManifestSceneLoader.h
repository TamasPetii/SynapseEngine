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

        void LoadAndMapModels(const std::vector<std::string>& modelManifest, std::vector<uint32_t>& outLocalToGlobalModels);
        void LoadAndMapAnimations(const std::vector<AnimationManifestEntry>& animManifest, const std::vector<uint32_t>& localToGlobalModels, std::vector<uint32_t>& outLocalToGlobalAnims);
        void LoadAndMapVideos(const std::vector<VideoManifestEntry>& videoManifest, std::vector<uint32_t>& outLocalToGlobalVideo);

        void LoadAndMapTextures(const std::vector<TextureManifestEntry>& texManifest, std::vector<uint32_t>& outLocalToGlobalTex);
        void RemapAndLoadMaterials(std::vector<MaterialManifestEntry>& matManifest, const std::vector<uint32_t>& localToGlobalTex, const std::vector<uint32_t>& localToGlobalVideo, std::vector<uint32_t>& outLocalToGlobalMats);

        void RemapModelComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalModels);
        void RemapAnimationComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalAnims);
        void RemapMaterialComponents(Scene& scene, const std::vector<uint32_t>& localToGlobalMats);

        void WakeUpEntities(Scene& scene);
    };
}