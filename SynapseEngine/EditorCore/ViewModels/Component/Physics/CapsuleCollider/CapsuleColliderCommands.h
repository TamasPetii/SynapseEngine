#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/ICapsuleColliderApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeCapsuleColliderRadiusCommand = ComponentChangeCommand<ICapsuleColliderApi, float, &ICapsuleColliderApi::SetCapsuleColliderRadius>;
    using ChangeCapsuleColliderHalfHeightCommand = ComponentChangeCommand<ICapsuleColliderApi, float, &ICapsuleColliderApi::SetCapsuleColliderHalfHeight>;
    using ChangeCapsuleColliderLocalOffsetCommand = ComponentChangeCommand<ICapsuleColliderApi, glm::vec3, &ICapsuleColliderApi::SetCapsuleColliderLocalOffset>;
}