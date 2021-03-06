/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Core/Json.hpp>
#include <sstream>

namespace Cm { namespace Core {

std::string JsonStringCharHexEscape(uint32_t x)
{
    std::stringstream s;
    uint8_t x0 = (x >> (3 * 8)) & 0xFF;
    uint8_t x1 = (x >> (2 * 8)) & 0xFF;
    uint8_t x2 = (x >> 8) & 0xFF;
    uint8_t x3 = x & 0xFF;
    s << std::hex << x0 << x1 << x2 << x3;
    return s.str();
}

JsonValue::~JsonValue()
{
}

JsonString::JsonString()
{
}

JsonString::JsonString(const std::string& value_) : value(value_)
{
}

void JsonString::Append(char c)
{
    value.append(1, c);
}

void JsonString::Append(const std::string& s)
{
    value.append(s);
}

std::string JsonStringCharStr(char c)
{
    switch (c)
    {
        case '\"': return "\\\"";
        case '\\': return "\\\\";
        case '\a': return "\\a";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case '\v': return "\\v";
        default:
        {
            if (c >= 32 && c <= 126)
            {
                return std::string(1, c);
            }
            else
            {
                return "\\u" + JsonStringCharHexEscape(uint32_t(c));
            }
        }
    }
}

std::string JsonString::ToString() const 
{
    std::string s = "\"";
    for (char c : value)
    {
        s.append(JsonStringCharStr(c));
    }
    s.append("\"");
    return s;
}

JsonNumber::JsonNumber() : value(0)
{
}

JsonNumber::JsonNumber(double value_) : value(value_)
{
}

std::string JsonNumber::ToString() const 
{
    std::stringstream s;
    s << value;
    return s.str();
}

JsonBool::JsonBool() : value(false)
{
}

JsonBool::JsonBool(bool value_) : value(value_)
{
}

std::string JsonBool::ToString() const 
{
    return value ? "true" : "false";
}

std::string JsonNull::ToString() const 
{
    return "null";
}

JsonObject::JsonObject()
{
}

void JsonObject::AddField(JsonString&& name, JsonValue* value)
{
    values.push_back(std::unique_ptr<JsonValue>(value));
    fields[name] = value;
}

JsonValue* JsonObject::GetField(const JsonString& name) const
{
    std::map<JsonString, JsonValue*>::const_iterator i = fields.find(name);
    if (i != fields.end())
    {
        return i->second;
    }
    return nullptr;
}

std::string JsonObject::ToString() const 
{
    std::string s = "{";
    bool first = true;
    for (const std::pair<JsonString, JsonValue*>& field : fields)
    {
        const JsonString& name = field.first;
        JsonValue* value = field.second;
        if (first)
        {
            first = false;
        }
        else
        {
            s.append(", ");
        }
        s.append(name.ToString());
        s.append(":");
        s.append(value->ToString());
    }
    s.append("}");
    return s;
}

JsonArray::JsonArray()
{
}

void JsonArray::AddItem(JsonValue* item)
{
    items.push_back(item);
    values.push_back(std::unique_ptr<JsonValue>(item));
}

std::string JsonArray::ToString() const 
{
    std::string s = "[";
    bool first = true;
    for (JsonValue* item : items)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            s.append(", ");
        }
        s.append(item->ToString());
    }
    s.append("]");
    return s;
}

} } // namespace Cm::Core
