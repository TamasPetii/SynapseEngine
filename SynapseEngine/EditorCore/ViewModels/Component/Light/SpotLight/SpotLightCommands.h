#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/ISpotLightApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    using ChangeSpotLightColorCommand = ComponentChangeCommand<ISpotLightApi, glm::vec3, &ISpotLightApi::SetLightColor>;
    using ChangeSpotLightStrengthCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetLightStrength>;
    using ChangeSpotLightRangeCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetLightRange>;
    using ChangeSpotLightWeakenCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetLightWeakenDistance>;
    using ChangeSpotLightInnerAngleCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetLightInnerAngle>;
    using ChangeSpotLightOuterAngleCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetLightOuterAngle>;
    using ChangeSpotLightShadowNearCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetShadowNearPlane>;
    using ChangeSpotLightShadowFarCommand = ComponentChangeCommand<ISpotLightApi, float, &ISpotLightApi::SetShadowFarPlane>;
}