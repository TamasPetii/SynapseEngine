#pragma once
#include "Engine/SynApi.h"
#include "Engine/Serialization/Stream/Input/IInputStream.h"
#include "IInputArchive.h"

namespace Syn
{
    class SYN_API BinaryInputArchive : public IInputArchive
    {
    public:
		explicit BinaryInputArchive(IInputStream& stream) : IInputArchive(stream) {}
        ~BinaryInputArchive() override = default;

        void EnterObject(const char* name) override {}
        void LeaveObject() override {}
        void EnterArray(const char* name, size_t& size) override;
        void LeaveArray() override {}

        void PropertyBool(const char* name, bool& value) override;
        void PropertyUint8(const char* name, uint8_t& value) override;
        void PropertyInt32(const char* name, int32_t& value) override;
        void PropertyUint32(const char* name, uint32_t& value) override;
        void PropertyInt64(const char* name, int64_t& value) override;
        void PropertyUint64(const char* name, uint64_t& value) override;
        void PropertyFloat(const char* name, float& value) override;
        void PropertyDouble(const char* name, double& value) override;
        void PropertyString(const char* name, std::string& value) override;
        void PropertyBytes(const char* name, void* data, size_t size) override;
    };
}