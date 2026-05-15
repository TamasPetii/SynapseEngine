#include "TransformModelLinkSystem.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include <print>

namespace Syn
{
    std::vector<TypeID> TransformModelLinkSystem::GetReadDependencies() const
    {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<ModelSystem>::ID
        };
    }

    std::vector<TypeID> TransformModelLinkSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TransformModelLinkSystem>::ID };
    }

    void TransformModelLinkSystem::OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow)
    {
        //std::println("[TransformModelLink] OnUploadToGpu hívva (Frame: {})", frameIndex);

        auto registry = scene->GetRegistry();
        auto [transformPool, modelPool] = registry->GetPools<TransformComponent, ModelComponent>();

        if (!transformPool || !modelPool) {
            //std::println("  -> [TransformModelLink] Kilép: Nincs transformPool vagy modelPool");
            return;
        }

        auto componentBufferManager = scene->GetComponentBufferManager();
        auto linkBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TransformModelLinkData, frameIndex);

        if (!linkBuffer.buffer) {
            //std::println("  -> [TransformModelLink] Kilép: Nincs linkBuffer");
            return;
        }

        if (_gpuLinkVersions.size() <= frameIndex)
            _gpuLinkVersions.resize(frameIndex + 1, 0);

        // We update the links if:
        // 1. A transform moved or was added (Transform INDEX_CHANGED_BIT or CHANGED_BIT)
        // 2. A model was changed or added (Model CHANGED_BIT)
        // 3. The static hierarchy was rebuilt (reordering occurred)
        bool transformDirty = transformPool->IsStateBitSet<CHANGED_BIT>() || transformPool->IsStateBitSet<INDEX_CHANGED_BIT>();
        bool modelDirty = modelPool->IsStateBitSet<CHANGED_BIT>() || modelPool->IsStateBitSet<INDEX_CHANGED_BIT>();
        bool staticDirty = !transformPool->GetStorage().GetDirtyStatics().empty();
        bool forceUpload = this->ShouldForceUpload();

        if (!transformDirty && !modelDirty && !staticDirty && !forceUpload) {
            //std::println("  -> [TransformModelLink] Kilép: Minden tiszta (nincs dirty flag)");
            return;
        }

        //std::println("  -> [TransformModelLink] Upload indul! Okok: TransformDirty: {}, ModelDirty: {}, StaticDirty: {}, Force: {}", transformDirty, modelDirty, staticDirty, forceUpload);

        auto linkBufferHandler = static_cast<TransformModelLinkGPU*>(linkBuffer.buffer->Map());

        auto processLink = [transformPool, modelPool, linkBuffer, linkBufferHandler, forceUpload](EntityID entity)
            {
                auto transformIndex = transformPool->GetMapping().Get(entity);
                auto& transformComp = transformPool->Get(entity);

                if (forceUpload || linkBuffer.versions[transformIndex] != transformComp.version)
                {
                    uint32_t modelIdx = NULL_INDEX;
                    if (modelPool->Has(entity))
                        modelIdx = modelPool->GetMapping().Get(entity);

                    linkBufferHandler[transformIndex] = { entity, modelIdx };
                    linkBuffer.versions[transformIndex] = transformComp.version;
                }
            };

        auto staticEntities = transformPool->GetStorage().GetStaticEntities();
        auto dynamicEntities = transformPool->GetStorage().GetDynamicEntities();
        auto streamEntities = transformPool->GetStorage().GetStreamEntities();

        if (!streamEntities.empty())
            subflow.for_each(streamEntities.begin(), streamEntities.end(), processLink);

        if (forceUpload || transformDirty || modelDirty)
            subflow.for_each(dynamicEntities.begin(), dynamicEntities.end(), processLink);

        if (forceUpload || staticDirty || modelDirty)
            subflow.for_each(staticEntities.begin(), staticEntities.end(), processLink);
    }
}