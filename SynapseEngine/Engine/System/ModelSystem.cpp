#include "ModelSystem.h"

namespace Syn
{
    std::vector<TypeID> ModelSystem::GetWriteDependencies() const { 
        return { TypeInfo<ModelSystem>::ID };
    }

    void ModelSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto modelPool = registry->GetPool<ModelComponent>();
        if (!modelPool) return;

        ParallelForEachIf<UPDATE_BIT>(modelPool, subflow, SystemPhaseNames::Update, [modelPool](EntityID entity) {
            auto& modelComponent = modelPool->Get(entity);

            modelPool->SetBit<CHANGED_BIT>(entity);
            modelComponent.version++;
            });
    }

    void ModelSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto modelPool = registry->GetPool<ModelComponent>();
        if (!modelPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::ModelData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<ModelComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [modelPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& modelComponent = modelPool->Get(entity);
            auto modelIndex = modelPool->GetMapping().Get(entity);

            if (componentBuffer.versions[modelIndex] != modelComponent.version)
            {
                componentBuffer.versions[modelIndex] = modelComponent.version;
                bufferHandler[modelIndex] = ModelComponentGPU(entity, modelComponent);
            }
            };

        ForEachStream(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
        {
            ForEachDynamic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
            ForEachStatic(modelPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        }
    }
}