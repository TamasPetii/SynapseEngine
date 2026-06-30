#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IRigidBodyApi.h"

namespace Syn
{
    using ChangeRigidBodyMassCommand = ComponentChangeCommand<IRigidBodyApi, float, &IRigidBodyApi::SetRigidBodyMass>;
    using ChangeRigidBodyFrictionCommand = ComponentChangeCommand<IRigidBodyApi, float, &IRigidBodyApi::SetRigidBodyFriction>;
    using ChangeRigidBodyRestitutionCommand = ComponentChangeCommand<IRigidBodyApi, float, &IRigidBodyApi::SetRigidBodyRestitution>;
}