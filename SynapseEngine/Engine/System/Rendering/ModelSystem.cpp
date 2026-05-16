#include "ModelSystem.h"
#include "MaterialSystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/FrameContext.h"

namespace Syn
{
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

        auto modelManager = ServiceLocator::GetModelManager();
        uint32_t currentVersion = modelManager->GetVersion();

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
        if (!modelPool) return;

        auto modelDataBuffer = componentBufferManager->GetComponentBuffer(BufferNames::ModelData, frameIndex);
        if (!modelDataBuffer.buffer) return;
        auto modelDataBufferHandler = static_cast<ModelComponentGPU*>(modelDataBuffer.buffer->Map());

        bool forceUpload = this->ShouldForceUpload();

        auto processUpload = [modelPool, modelDataBuffer, modelDataBufferHandler, forceUpload](EntityID entity) {
            auto& modelComponent = modelPool->Get(entity);
            auto modelIndex = modelPool->GetMapping().Get(entity);

            if (forceUpload || modelDataBuffer.versions[modelIndex] != modelComponent.version)
            {
                modelDataBuffer.versions[modelIndex] = modelComponent.version;
                modelDataBufferHandler[modelIndex] = ModelComponentGPU(entity, modelComponent);
            }

            };

        ForEachStream(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}