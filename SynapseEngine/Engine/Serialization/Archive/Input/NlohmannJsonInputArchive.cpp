#include "NlohmannJsonInputArchive.h"
#include <stdexcept>

namespace Syn
{
    template<typename T>
    void ReadValue(nlohmann::json* current, const char* name, T& value) {
        value = (*current).at(name).get<T>();
    }

	NlohmannJsonInputArchive::NlohmannJsonInputArchive(IInputStream& stream) 
        : IJsonInputArchive(stream)
        , _current(nullptr)
    {
        
    }

    void NlohmannJsonInputArchive::NlohmannJsonInputArchive::Deserialize()
    {
        size_t size = _stream.GetSize();
        if (size > 0)
        {
            std::string jsonString(size, '\0');
            _stream.ReadRaw(jsonString.data(), size);

            _root = nlohmann::json::parse(jsonString);
            _current = &_root;

            _stack.clear();
            _stack.push_back(_current);
        }
    }

    void NlohmannJsonInputArchive::EnterObject(const char* name)
    {
        _current = &((*_current).at(name));
        _stack.push_back(_current);
    }

    void NlohmannJsonInputArchive::LeaveObject()
    {
        _stack.pop_back();
        _current = _stack.back();
    }

    void NlohmannJsonInputArchive::EnterArray(const char* name, size_t& size)
    {
        _current = &((*_current).at(name));
        _stack.push_back(_current);
        size = _current->size();
    }

    void NlohmannJsonInputArchive::LeaveArray()
    {
        _stack.pop_back();
        _current = _stack.back();
    }



    void NlohmannJsonInputArchive::PropertyBool(const char* name, bool& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint8(const char* name, uint8_t& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyInt32(const char* name, int32_t& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint32(const char* name, uint32_t& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyInt64(const char* name, int64_t& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint64(const char* name, uint64_t& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyFloat(const char* name, float& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyDouble(const char* name, double& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyString(const char* name, std::string& value) { 
        ReadValue(_current, name, value);
    }

    void NlohmannJsonInputArchive::PropertyBytes(const char* name, void* data, size_t size)
    {
        auto vec = (*_current).at(name).get<std::vector<uint8_t>>();
        size_t copySize = std::min(size, vec.size());
        if (data && copySize > 0) {
            std::memcpy(data, vec.data(), copySize);
        }
    }
}