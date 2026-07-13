#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IConvexColliderApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeConvexColliderLocalOffsetCommand = ComponentChangeCommand<IConvexColliderApi, glm::vec3, &IConvexColliderApi::SetConvexColliderLocalOffset>;
    using ChangeConvexColliderTargetLodCommand = ComponentChangeCommand<IConvexColliderApi, uint32_t, &IConvexColliderApi::SetConvexColliderTargetLodLevel>;
}