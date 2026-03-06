#pragma once
#include "IModelCooker.h"

namespace Syn
{
    class SYN_API DefaultModelCooker : public IModelCooker
    {
    public:
        DefaultModelCooker() = default;
        ~DefaultModelCooker() override = default;

        CookedModel Cook(RawModel&& rawModel) override;
    private:
        CookedMesh CookMesh(RawMesh&& rawMesh);
    };
}