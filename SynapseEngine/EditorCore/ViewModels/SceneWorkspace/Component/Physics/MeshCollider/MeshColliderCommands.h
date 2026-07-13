#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IMeshColliderApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeMeshColliderLocalOffsetCommand = ComponentChangeCommand<IMeshColliderApi, glm::vec3, &IMeshColliderApi::SetMeshColliderLocalOffset>;
    using ChangeMeshColliderTargetLodCommand = ComponentChangeCommand<IMeshColliderApi, uint32_t, &IMeshColliderApi::SetMeshColliderTargetLodLevel>;
}