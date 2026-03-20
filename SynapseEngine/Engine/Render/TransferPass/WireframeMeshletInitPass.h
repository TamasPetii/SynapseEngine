#pragma once
#include "Engine/SynApi.h"
#include "TransferPass.h"

namespace Syn
{
    class SYN_API WireframeMeshletInitPass : public TransferPass
    {
    public:
        std::string GetName() const override { return "WireframeMeshletInitPass"; }
    protected:
        void Transfer(const RenderContext& context) override;
    };
}