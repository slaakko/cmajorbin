/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_BIND_VALUE_INCLUDED
#define CM_BIND_VALUE_INCLUDED
#include <Cm.Sym/Reader.hpp>
#include <Cm.Sym/Writer.hpp>
#include <Cm.Sym/VariableSymbol.hpp>
#include <Cm.Ser/BinaryReader.hpp>
#include <Cm.Ser/BinaryWriter.hpp>
#include <Cm.Parsing/Scanner.hpp>
#include <Cm.Util/TextUtils.hpp>
#include <Ir.Intf/Object.hpp>
#include <stdint.h>

namespace Cm { namespace Sym {

using Cm::Parsing::Span;

enum class ValueType : uint8_t
{
    none, boolValue, charValue, wcharValue, ucharValue, sbyteValue, byteValue, shortValue, ushortValue, intValue, uintValue, longValue, ulongValue, floatValue, doubleValue, nullValue, stringValue, 
    charPtrValue, max
};

std::string ValueTypeStr(ValueType valueType);
ValueType GetValueTypeFor(SymbolType symbolType, bool dontThrow);
ValueType GetCommonType(ValueType left, ValueType right);
TypeSymbol* GetTypeSymbol(ValueType valueType, SymbolTable& symbolTable);

class Value
{
public:
    virtual ~Value();
    virtual ValueType GetValueType() const = 0;
    virtual Value* Clone() const = 0;
    virtual void Read(Reader& reader) = 0;
    virtual void Write(Writer& writer) = 0;
    virtual Value* As(ValueType targetType, bool cast, const Span& span) const = 0;
    virtual void Inc(const Span& span) = 0;
    virtual void Dec(const Span& span) = 0;
    virtual Value* Deref(const Span& span) const = 0;
    virtual Ir::Intf::Object* CreateIrObject() const = 0;
    virtual bool IsScopedValue() const { return false; }
    virtual bool IsFuntionGroupValue() const { return false; }
    virtual bool IsVariableValue() const { return false; }
    virtual bool IsNull() const { return false; }
    virtual std::string ToString() const { return ""; }
};

class BoolValue : public Value
{
public:
    typedef bool OperandType;
    BoolValue();
    BoolValue(bool value_);
    ValueType GetValueType() const override { return ValueType::boolValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    Value* Deref(const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Ir::Intf::Object* CreateIrObject() const override;
    bool Value() const { return value; }
    std::string ToString() const override { return value ? "true" : "false"; }
private:
    bool value;
};

class CharValue : public Value
{
public:
    typedef char OperandType;
    CharValue();
    CharValue(char value_);
    ValueType GetValueType() const override { return ValueType::charValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    Value* Deref(const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Ir::Intf::Object* CreateIrObject() const override;
    char Value() const { return value; }
    std::string ToString() const override { return "'" + Cm::Util::CharStr(value) + "'"; }
private:
    char value;
};

class WCharValue : public Value
{
public:
    typedef uint16_t OperandType;
    WCharValue();
    WCharValue(uint16_t value_);
    ValueType GetValueType() const override { return ValueType::wcharValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    Value* Deref(const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint16_t Value() const { return value; }
    std::string ToString() const override { return "'" + Cm::Util::WCharStr(value) + "'"; }
private:
    uint16_t value;
};

class UCharValue : public Value
{
public:
    typedef uint32_t OperandType;
    UCharValue();
    UCharValue(uint32_t value_);
    ValueType GetValueType() const override { return ValueType::ucharValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    Value* Deref(const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint32_t Value() const { return value; }
    std::string ToString() const override { return "'" + Cm::Util::UCharStr(value) + "'"; }
private:
    uint32_t value;
};

class SByteValue : public Value
{
public:
    typedef int8_t OperandType;
    SByteValue();
    SByteValue(int8_t value_);
    ValueType GetValueType() const override { return ValueType::sbyteValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    int8_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    int8_t value;
};

class ByteValue : public Value
{
public:
    typedef uint8_t OperandType;
    ByteValue();
    ByteValue(uint8_t value_);
    ValueType GetValueType() const override { return ValueType::byteValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint8_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    uint8_t value;
};

class ShortValue : public Value
{
public:
    typedef int16_t OperandType;
    ShortValue();
    ShortValue(int16_t value_);
    ValueType GetValueType() const override { return ValueType::shortValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    int16_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    int16_t value;
};

class UShortValue : public Value
{
public:
    typedef uint16_t OperandType;
    UShortValue();
    UShortValue(uint16_t value_);
    ValueType GetValueType() const override { return ValueType::ushortValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint16_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    uint16_t value;
};

class IntValue : public Value
{
public:
    typedef int32_t OperandType;
    IntValue();
    IntValue(int32_t value_);
    ValueType GetValueType() const override { return ValueType::intValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    int32_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    int32_t value;
};

class UIntValue : public Value
{
public:
    typedef uint32_t OperandType;
    UIntValue();
    UIntValue(uint32_t value_);
    ValueType GetValueType() const override { return ValueType::uintValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint32_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    uint32_t value;
};

class LongValue : public Value
{
public:
    typedef int64_t OperandType;
    LongValue();
    LongValue(int64_t value_);
    ValueType GetValueType() const override { return ValueType::longValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    int64_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    int64_t value;
};

class ULongValue : public Value
{
public:
    typedef uint64_t OperandType;
    ULongValue();
    ULongValue(uint64_t value_);
    ValueType GetValueType() const override { return ValueType::ulongValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    uint64_t Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    uint64_t value;
};

class FloatValue : public Value
{
public:
    typedef float OperandType;
    FloatValue();
    FloatValue(float value_);
    ValueType GetValueType() const override { return ValueType::floatValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    float Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    float value;
};

class DoubleValue : public Value
{
public:
    typedef double OperandType;
    DoubleValue();
    DoubleValue(double value_);
    ValueType GetValueType() const override { return ValueType::doubleValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    double Value() const { return value; }
    std::string ToString() const override { return std::to_string(value); }
private:
    double value;
};

class NullValue : public Value
{
public:
    NullValue();
    ValueType GetValueType() const override { return ValueType::nullValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    bool IsNull() const override { return true; }
    void SetType(Cm::Sym::TypeSymbol* type_) { type = type_; }
    std::string ToString() const override { return "null"; }
private:
    Cm::Sym::TypeSymbol* type;
};

class StringValue : public Value
{
public:
    StringValue();
    StringValue(const std::string& value_);
    ValueType GetValueType() const override { return ValueType::stringValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override;
    void Dec(const Span& span) override;
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    const std::string& Value() const { return value; }
    std::string ToString() const override { return "\"" + Cm::Util::StringStr(value) + "\""; }
private:
    std::string value;
};

class CharPtrValue : public Value
{
public:
    typedef const char* OperandType;
    CharPtrValue();
    CharPtrValue(const char* value_);
    ValueType GetValueType() const override { return ValueType::charPtrValue; }
    Value* Clone() const override;
    void Read(Reader& reader) override;
    void Write(Writer& writer) override;
    Value* As(ValueType targetType, bool cast, const Span& span) const override;
    void Inc(const Span& span) override { ++value; }
    void Dec(const Span& span) override { --value; }
    Value* Deref(const Span& span) const override;
    Ir::Intf::Object* CreateIrObject() const override;
    const char* Value() const { return value; }
private:
    const char* value;
};

class EvaluationStack
{
public:
    void Push(Cm::Sym::Value* value);
    Cm::Sym::Value* Pop();
    int Count() const { return int(stack.size()); }
private:
    std::stack<std::unique_ptr<Cm::Sym::Value>> stack;
};

} } // namespace Cm::Sym

#endif // CM_BIND_VALUE_INCLUDED

