#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IBoxColliderApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeBoxColliderHalfExtentsCommand = ComponentChangeCommand<IBoxColliderApi, glm::vec3, &IBoxColliderApi::SetBoxColliderHalfExtents>;
    using ChangeBoxColliderLocalOffsetCommand = ComponentChangeCommand<IBoxColliderApi, glm::vec3, &IBoxColliderApi::SetBoxColliderLocalOffset>;
}