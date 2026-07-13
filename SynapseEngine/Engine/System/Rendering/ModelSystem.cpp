#include "ModelSystem.h"
#include "MaterialSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/FrameContext.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn
{
    constexpr bool ENABLE_DEBUG_LOGGING = false;

    std::vector<TypeID> ModelSystem::GetReadDependencies() const {
        return { 
            TypeInfo<MaterialSystem>::ID
        };
    }

    std::vector<TypeID> ModelSystem::GetWriteDependencies() const { 
        return { TypeInfo<ModelSystem>::ID };
    }

    void ModelSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        if (!modelPool) return;

        uint32_t currentVersion = scene->GetSystemContext().modelManagerVersion;

        this->EmplaceTask(subflow, SystemPhaseNames::Update, [this, scene, currentVersion]() {
            if (_lastModelManagerVersion != currentVersion) {
                _lastModelManagerVersion = currentVersion;
                this->SetFramesToUpload(ServiceLocator::GetFrameContext()->framesInFlight);
            }
            });

        ParallelForEachIf<UPDATE_BIT>(modelPool, subflow, SystemPhaseNames::Update, [modelPool](EntityID entity) {
            auto& modelComponent = modelPool->Get(entity);

            modelPool->SetBit<CHANGED_BIT>(entity);
            modelComponent.version++;
            });
    }

    void ModelSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto modelPool= registry->GetPool<ModelComponent>();
        auto tagPool = registry->GetPool<TagComponent>();
        if (!modelPool) return;

        auto modelDataBuffer = componentBufferManager->GetComponentBuffer(BufferNames::ModelData, frameIndex);
        if (!modelDataBuffer.buffer) return;
        auto modelDataBufferHandler = static_cast<ModelComponentGPU*>(modelDataBuffer.buffer->Map());

        bool forceUpload = this->ShouldForceUpload();

        auto processUpload = [modelPool, tagPool, modelDataBuffer, modelDataBufferHandler, forceUpload, scene](EntityID entity) {
            auto& modelComponent = modelPool->Get(entity);
            auto modelIndex = modelPool->GetMapping().Get(entity);

            if (forceUpload || modelDataBuffer.versions[modelIndex] != modelComponent.version)
            {
                if constexpr (ENABLE_DEBUG_LOGGING) {
                    std::string name = "Unknown";
                    if (tagPool && tagPool->Has(entity)) name = tagPool->Get(entity).name;

                    Info("[ModelSystem UPLOAD] Entity: {} ({}) | Material offset: {}", (uint32_t)entity, name, modelComponent.materialOffset);
                }

                modelDataBuffer.versions[modelIndex] = modelComponent.version;
                modelDataBufferHandler[modelIndex] = ModelComponentGPU(entity, modelComponent);
            }

            };

        ForEachStream(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic) 
            ForEachDynamic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}