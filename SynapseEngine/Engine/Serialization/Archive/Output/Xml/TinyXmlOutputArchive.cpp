#include "TinyXmlOutputArchive.h"
#include <string>

namespace Syn
{
    TinyXmlOutputArchive::TinyXmlOutputArchive(IOutputStream& stream) : IXmlOutputArchive(stream)
    {
        _doc.InsertFirstChild(_doc.NewDeclaration());
        _stack.push_back({ &_doc, false });
    }

    std::string TinyXmlOutputArchive::ToString() const
    {
        tinyxml2::XMLPrinter printer;
        _doc.Print(&printer);
        return std::string(printer.CStr());
    }

    void TinyXmlOutputArchive::Serialize()
    {
        std::string xmlStr = ToString();
        _stream.WriteRaw(xmlStr.data(), xmlStr.size());
    }

    void TinyXmlOutputArchive::EnterObject(const char* name)
    {
        auto& parentCtx = _stack.back();
        const char* tagName = parentCtx.isArray ? "item" : name;

        tinyxml2::XMLElement* el = _doc.NewElement(tagName);
        parentCtx.node->InsertEndChild(el);
        _stack.push_back({ el, false });
    }

    void TinyXmlOutputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void TinyXmlOutputArchive::EnterArray(const char* name, uint32_t size)
    {
        auto& parentCtx = _stack.back();
        const char* tagName = parentCtx.isArray ? "item" : name;

        tinyxml2::XMLElement* el = _doc.NewElement(tagName);
        el->SetAttribute("size", static_cast<unsigned int>(size));
        parentCtx.node->InsertEndChild(el);
        _stack.push_back({ el, true });
    }

    void TinyXmlOutputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    template<typename T>
    void TinyXmlOutputArchive::WriteValue(const char* name, T value)
    {
        auto& parentCtx = _stack.back();
        const char* tagName = parentCtx.isArray ? "item" : name;

        tinyxml2::XMLElement* el = _doc.NewElement(tagName);
        el->SetText(value);
        parentCtx.node->InsertEndChild(el);
    }

    void TinyXmlOutputArchive::PropertyBool(const char* name, bool value) { 
        WriteValue(name, value);
    }

    void TinyXmlOutputArchive::PropertyUint8(const char* name, uint8_t value) { 
        WriteValue(name, static_cast<unsigned int>(value));
    }

    void TinyXmlOutputArchive::PropertyUint16(const char* name, uint16_t value) {
        WriteValue(name, static_cast<unsigned int>(value));
    }

    void TinyXmlOutputArchive::PropertyInt32(const char* name, int32_t value) { 
        WriteValue(name, value);
    }

    void TinyXmlOutputArchive::PropertyUint32(const char* name, uint32_t value) { 
        WriteValue(name, value);
    }

    void TinyXmlOutputArchive::PropertyInt64(const char* name, int64_t value) { 
        WriteValue(name, static_cast<int64_t>(value));
    }

    void TinyXmlOutputArchive::PropertyUint64(const char* name, uint64_t value) { 
        WriteValue(name, static_cast<uint64_t>(value));
    }

    void TinyXmlOutputArchive::PropertyFloat(const char* name, float value) { 
        WriteValue(name, value);
    }

    void TinyXmlOutputArchive::PropertyDouble(const char* name, double value) { 
        WriteValue(name, value);
    }

    void TinyXmlOutputArchive::PropertyString(const char* name, const std::string& value) {
        WriteValue(name, value.c_str());
    }

    void TinyXmlOutputArchive::PropertyBytes(const char* name, const void* data, size_t size)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        std::string hexStr;
        hexStr.reserve(size * 2);
        const char* hexChars = "0123456789ABCDEF";
        for (size_t i = 0; i < size; ++i) {
            hexStr.push_back(hexChars[(bytes[i] >> 4) & 0x0F]);
            hexStr.push_back(hexChars[bytes[i] & 0x0F]);
        }
        WriteValue(name, hexStr.c_str());
    }
}