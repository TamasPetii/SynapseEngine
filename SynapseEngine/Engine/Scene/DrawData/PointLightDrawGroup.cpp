#include "PointLightDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn
{
    PointLightDrawGroup::PointLightDrawGroup(uint32_t frameCount)
    {
        auto modelManager = ServiceLocator::GetModelManager();
        auto cube = modelManager->GetResource(MeshSourceNames::Cube);
        auto sphere = modelManager->GetResource(MeshSourceNames::Sphere);

        VkDrawIndirectCommand sphereCmdTemplate{};
        sphereCmdTemplate.vertexCount = sphere->baseDrawCommands[0].traditionalCmd.vertexCount;
        sphereCmdTemplate.instanceCount = 0;
        sphereCmdTemplate.firstVertex = sphere->baseDrawCommands[0].traditionalCmd.firstVertex;
        sphereCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand aabbCmdTemplate{};
        aabbCmdTemplate.vertexCount = cube->baseDrawCommands[0].traditionalCmd.vertexCount;
        aabbCmdTemplate.instanceCount = 0;
        aabbCmdTemplate.firstVertex = cube->baseDrawCommands[0].traditionalCmd.firstVertex;
        aabbCmdTemplate.firstInstance = 0;

        VkDrawIndirectCommand billboardCmdTemplate{};
        billboardCmdTemplate.vertexCount = 6;
        billboardCmdTemplate.instanceCount = 0;
        billboardCmdTemplate.firstVertex = 0;
        billboardCmdTemplate.firstInstance = 0;

        cmdTemplate = aabbCmdTemplate;

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        indirectBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), usage });
        indirectBuffer.UpdateCapacityAll(1);

        sphereSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), usage });
        sphereSingleCmdBuffer.UpdateCapacityAll(1);

        aabbSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), usage });
        aabbSingleCmdBuffer.UpdateCapacityAll(1);

        billboardSingleCmdBuffer.Initialize({ BufferStrategy::MappedOnly, frameCount, sizeof(VkDrawIndirectCommand), usage });
        billboardSingleCmdBuffer.UpdateCapacityAll(1);

        for (uint32_t i = 0; i < frameCount; ++i) {
            sphereSingleCmdBuffer.GetMapped(i)->Write(&sphereCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            aabbSingleCmdBuffer.GetMapped(i)->Write(&aabbCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
            billboardSingleCmdBuffer.GetMapped(i)->Write(&billboardCmdTemplate, sizeof(VkDrawIndirectCommand), 0);
        }
    }
}