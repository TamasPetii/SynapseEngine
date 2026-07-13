#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/ISphereColliderApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeSphereColliderRadiusCommand = ComponentChangeCommand<ISphereColliderApi, float, &ISphereColliderApi::SetSphereColliderRadius>;
    using ChangeSphereColliderLocalOffsetCommand = ComponentChangeCommand<ISphereColliderApi, glm::vec3, &ISphereColliderApi::SetSphereColliderLocalOffset>;
}