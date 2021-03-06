/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Sym/Value.hpp>
#include <Cm.Sym/Exception.hpp>
#include <Cm.Sym/SymbolTable.hpp>
#include <Cm.Sym/BasicTypeSymbol.hpp>
#include <Cm.IrIntf/Rep.hpp>
#include <Cm.Sym/TypeSymbol.hpp>

namespace Cm { namespace Sym {

ValueType valueTypes[uint8_t(Cm::Sym::SymbolType::maxSymbol)] =
{
    ValueType::boolValue, ValueType::charValue, ValueType::wcharValue, ValueType::ucharValue, ValueType::none,
    ValueType::sbyteValue, ValueType::byteValue, ValueType::shortValue, ValueType::ushortValue, ValueType::intValue, ValueType::uintValue, ValueType::longValue, ValueType::ulongValue,
    ValueType::floatValue, ValueType::doubleValue, ValueType::nullValue, ValueType::none,
    ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
    ValueType::none, ValueType::none, ValueType::none
};

ValueType GetValueTypeFor(SymbolType symbolType, bool dontThrow)
{
    ValueType valueType = valueTypes[uint8_t(symbolType)];
    if (valueType == ValueType::none && !dontThrow)
    {
        throw std::runtime_error("invalid basic value type for symbol type");
    }
    return valueType;
}

ValueType commonType[uint8_t(Cm::Sym::ValueType::max)][uint8_t(Cm::Sym::ValueType::max)] = 
{
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::boolValue, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::charValue, ValueType::wcharValue, ValueType::ucharValue, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::wcharValue, ValueType::wcharValue, ValueType::ucharValue, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::ucharValue, ValueType::ucharValue, ValueType::ucharValue, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::sbyteValue, ValueType::shortValue, ValueType::shortValue, ValueType::intValue, ValueType::intValue, ValueType::longValue, ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::shortValue, ValueType::byteValue, ValueType::shortValue, ValueType::ushortValue, ValueType::intValue, ValueType::uintValue, ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::shortValue, ValueType::shortValue, ValueType::shortValue, ValueType::intValue, ValueType::intValue, ValueType::longValue, ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::intValue, ValueType::ushortValue, ValueType::intValue, ValueType::ushortValue, ValueType::intValue, ValueType::uintValue, ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::longValue, ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::longValue, ValueType::uintValue, ValueType::longValue, ValueType::uintValue, ValueType::longValue, ValueType::uintValue, ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::ulongValue, ValueType::none, ValueType::ulongValue, ValueType::none, ValueType::ulongValue, ValueType::none, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::nullValue, ValueType::none, ValueType::charPtrValue },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::stringValue, ValueType::none },
    { ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::charPtrValue , ValueType::none, ValueType::charPtrValue }
};

ValueType GetCommonType(ValueType left, ValueType right)
{
    return commonType[uint8_t(left)][uint8_t(right)];
}

const char* valueTypeStr[uint8_t(ValueType::max)] =
{
    "", "bool", "char", "wchar", "uchar", "sbyte", "byte", "short", "ushort", "int", "uint", "long", "ulong", "float", "double", "@nullptrtype", "string", "char*"
};

std::string ValueTypeStr(ValueType valueType)
{
    return valueTypeStr[uint8_t(valueType)];
}

TypeSymbol* GetTypeSymbol(ValueType valueType, SymbolTable& symbolTable)
{
    switch (valueType)
    {
        case ValueType::boolValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::boolId));
        case ValueType::charValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::charId));
        case ValueType::wcharValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::wcharId));
        case ValueType::ucharValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::ucharId));
        case ValueType::sbyteValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::sbyteId));
        case ValueType::byteValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::byteId));
        case ValueType::shortValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::shortId));
        case ValueType::ushortValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::ushortId));
        case ValueType::intValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::intId));
        case ValueType::uintValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::uintId));
        case ValueType::longValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::longId));
        case ValueType::ulongValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::ulongId));
        case ValueType::floatValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::floatId));
        case ValueType::doubleValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::doubleId));
        case ValueType::nullValue: return symbolTable.GetTypeRepository().GetTypeNothrow(GetBasicTypeId(Cm::Sym::ShortBasicTypeId::nullPtrId));
        case ValueType::stringValue: return symbolTable.GetTypeRepository().MakeConstCharPtrType(Span());
        case ValueType::charPtrValue: return symbolTable.GetTypeRepository().MakeConstCharPtrType(Span());
    }
    return nullptr;
}

Value::~Value()
{
}

BoolValue::BoolValue() : value(false)
{
}

BoolValue::BoolValue(bool value_) : value(value_)
{
}

Value* BoolValue::Clone() const
{
    return new BoolValue(value);
}

void BoolValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadBool();
}

void BoolValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* BoolValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            return new BoolValue(value);
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

void BoolValue::Inc(const Span& span)
{
    throw Exception("cannot increment a Boolean", span);
}

void BoolValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a Boolean", span);
}

Value* BoolValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a Boolean", span);
}

Ir::Intf::Object* BoolValue::CreateIrObject() const
{
    return value ? Cm::IrIntf::True() : Cm::IrIntf::False();
}

CharValue::CharValue() : value('\0')
{
}

CharValue::CharValue(char value_) : value(value_)
{
}

Value* CharValue::Clone() const
{
    return new CharValue(value);
}

void CharValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadChar();
}

void CharValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* CharValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            return new CharValue(value);
        }
        case ValueType::wcharValue:
        {
            return new WCharValue(value);
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(value);
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* CharValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a char", span);
}

void CharValue::Inc(const Span& span)
{
    throw Exception("cannot increment a char", span);
}

void CharValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a char", span);
}

Ir::Intf::Object* CharValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateCharConstant(value);
}

WCharValue::WCharValue() : value(0)
{
}

WCharValue::WCharValue(uint16_t value_) : value(value_)
{
}

Value* WCharValue::Clone() const
{
    return new WCharValue(value);
}

void WCharValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadUShort();
}

void WCharValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* WCharValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            return new WCharValue(value);
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(value);
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* WCharValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a wchar", span);
}

void WCharValue::Inc(const Span& span)
{
    throw Exception("cannot increment a wchar", span);
}

void WCharValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a wchar", span);
}

Ir::Intf::Object* WCharValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI16Constant(value);
}

UCharValue::UCharValue() : value()
{
}

UCharValue::UCharValue(uint32_t value_) : value(value_)
{
}

Value* UCharValue::Clone() const
{
    return new UCharValue(value);
}

void UCharValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadUInt();
}

void UCharValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* UCharValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(value);
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(static_cast<double>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* UCharValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a uchar", span);
}

void UCharValue::Inc(const Span& span)
{
    throw Exception("cannot increment a uchar", span);
}

void UCharValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a uchar", span);
}

Ir::Intf::Object* UCharValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI32Constant(value);
}

SByteValue::SByteValue() : value(0)
{
}

SByteValue::SByteValue(int8_t value_) : value(value_)
{
}

Value* SByteValue::Clone() const
{
    return new SByteValue(value);
}

void SByteValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadSByte();
}

void SByteValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* SByteValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            return new SByteValue(value);
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* SByteValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference an sbyte", span);
}

Ir::Intf::Object* SByteValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateI8Constant(value);
}

ByteValue::ByteValue() : value(0)
{
}

ByteValue::ByteValue(uint8_t value_) : value(value_)
{
}

Value* ByteValue::Clone() const
{
    return new ByteValue(value);
}

void ByteValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadByte();
}

void ByteValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* ByteValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            return new ByteValue(value);
        }
        case ValueType::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueType::ushortValue:
        {
            return new UShortValue(value);
        }
        case ValueType::intValue:
        {
            return new IntValue(value);
        }
        case ValueType::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* ByteValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a byte", span);
}

Ir::Intf::Object* ByteValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI8Constant(value);
}

ShortValue::ShortValue() : value(0)
{
}

ShortValue::ShortValue(int16_t value_) : value(value_)
{
}

Value* ShortValue::Clone() const
{
    return new ShortValue(value);
}

void ShortValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadShort();
}

void ShortValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* ShortValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            return new ShortValue(value);
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* ShortValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a short", span);
}

Ir::Intf::Object* ShortValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateI16Constant(value);
}

UShortValue::UShortValue() : value(0)
{
}

UShortValue::UShortValue(uint16_t value_) : value(value_)
{
}

Value* UShortValue::Clone() const
{
    return new UShortValue(value);
}

void UShortValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadUShort();
}

void UShortValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* UShortValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            return new UShortValue(value);
        }
        case ValueType::intValue:
        {
            return new IntValue(value);
        }
        case ValueType::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* UShortValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a ushort", span);
}

Ir::Intf::Object* UShortValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI16Constant(value);
}

IntValue::IntValue() : value(0)
{
}

IntValue::IntValue(int32_t value_) : value(value_)
{
}

Value* IntValue::Clone() const
{
    return new IntValue(value);
}

void IntValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadInt();
}

void IntValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* IntValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        { 
            throw Exception("invalid conversion", span);
        }
    }
}

Value* IntValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference an int", span);
}

Ir::Intf::Object* IntValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateI32Constant(value);
}

UIntValue::UIntValue() : value(0)
{
}

UIntValue::UIntValue(uint32_t value_) : value(value_)
{
}

Value* UIntValue::Clone() const
{
    return new UIntValue(value);
}

void UIntValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadUInt();
}

void UIntValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* UIntValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            return new UIntValue(value);
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* UIntValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a uint", span);
}

Ir::Intf::Object* UIntValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI32Constant(value);
}

LongValue::LongValue() : value(0)
{
}

LongValue::LongValue(int64_t value_) : value(value_)
{
}

Value* LongValue::Clone() const
{
    return new LongValue(value);
}

void LongValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadLong();
}

void LongValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* LongValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* LongValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a long", span);
}

Ir::Intf::Object* LongValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateI64Constant(value);
}

ULongValue::ULongValue() : value(0)
{
}

ULongValue::ULongValue(uint64_t value_) : value(value_)
{
}

Value* ULongValue::Clone() const
{
    return new ULongValue(value);
}

void ULongValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadULong();
}

void ULongValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* ULongValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

Value* ULongValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a ulong", span);
}

Ir::Intf::Object* ULongValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateUI64Constant(value);
}

FloatValue::FloatValue() : value(0.0f)
{
}

FloatValue::FloatValue(float value_) : value(value_)
{
}

Value* FloatValue::Clone() const
{
    return new FloatValue(value);
}

void FloatValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadFloat();
}

void FloatValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* FloatValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

void FloatValue::Inc(const Span& span)
{
    throw Exception("cannot increment a float", span);
}

void FloatValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a float", span);
}

Value* FloatValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a float", span);
}

Ir::Intf::Object* FloatValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateFloatConstant(value);
}

DoubleValue::DoubleValue() : value(0.0)
{
}

DoubleValue::DoubleValue(double value_) : value(value_)
{
}

Value* DoubleValue::Clone() const
{
    return new DoubleValue(value);
}

void DoubleValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadDouble();
}

void DoubleValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* DoubleValue::As(ValueType targetType, bool cast, const Span& span) const
{
    switch (targetType) 
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(static_cast<bool>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(static_cast<char>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(static_cast<uint32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(static_cast<int8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(static_cast<uint8_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(static_cast<int16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(static_cast<uint16_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(static_cast<int32_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(static_cast<int64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(static_cast<uint64_t>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(static_cast<float>(value));
            }
            else
            {
                throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
            }
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(value);
        }
        default:
        {
            throw Exception("invalid conversion", span);
        }
    }
}

void DoubleValue::Inc(const Span& span)
{
    throw Exception("cannot increment a double", span);
}

void DoubleValue::Dec(const Span& span)
{
    throw Exception("cannot decrement a double", span);
}

Value* DoubleValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a double", span);
}

Ir::Intf::Object* DoubleValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateDoubleConstant(value);
}

NullValue::NullValue() : type(nullptr)
{
}

Value* NullValue::Clone() const
{
    return new NullValue();
}

void NullValue::Read(Reader& reader)
{
    bool hasType = reader.GetBinaryReader().ReadBool();
    if (hasType)
    {
        uint32_t sid = reader.GetBinaryReader().ReadUInt();
        Symbol* symbol = reader.GetSymbolTable().GetSymbol(sid);
        if (!symbol)
        {
            throw std::runtime_error("could not get type symbol for sid " + std::to_string(sid) + " from symbol table");
        }
        if (symbol->IsTypeSymbol())
        {
            type = static_cast<TypeSymbol*>(symbol);
        }
        else
        {
            throw std::runtime_error("type symbol expected");
        }
    }
}

void NullValue::Write(Writer& writer)
{
    bool hasType = type != nullptr;
    writer.GetBinaryWriter().Write(hasType);
    if (hasType)
    {
        uint32_t sid = type->Sid();
        type->DoSerialize();
        writer.GetBinaryWriter().Write(sid);
    }
}

Value* NullValue::As(ValueType targetType, bool cast, const Span& span) const
{
    if (targetType == ValueType::charPtrValue)
    {
        return new CharPtrValue(nullptr);
    }
    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType), span);
}

void NullValue::Inc(const Span& span)
{
    throw Exception("cannot increment null value", span);
}

void NullValue::Dec(const Span& span)
{
    throw Exception("cannot decrement null value", span);
}

Value* NullValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a null value", span);
}

Ir::Intf::Object* NullValue::CreateIrObject() const
{
    Ir::Intf::Type* irType = nullptr;
    if (type)
    {
        irType = type->GetIrType();
    }
    else
    {
        irType = Cm::IrIntf::Pointer(Ir::Intf::GetFactory()->GetI1(), 1);
    }
    return Cm::IrIntf::Null(irType);
}

StringValue::StringValue() : value("")
{
}

StringValue::StringValue(const std::string& value_) : value(value_)
{
}

Value* StringValue::Clone() const 
{
    return new StringValue(value);
}

void StringValue::Read(Reader& reader)
{
    value = reader.GetBinaryReader().ReadString();
}

void StringValue::Write(Writer& writer)
{
    writer.GetBinaryWriter().Write(value);
}

Value* StringValue::As(ValueType targetType, bool cast, const Span& span) const
{
    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType), span);
}

void StringValue::Inc(const Span& span)
{
    throw Exception("cannot increment string value", span);
}

void StringValue::Dec(const Span& span)
{
    throw Exception("cannot decrement string value", span);
}

Value* StringValue::Deref(const Span& span) const
{
    throw Exception("cannot dereference a string value", span);
}

Ir::Intf::Object* StringValue::CreateIrObject() const
{
    return Cm::IrIntf::CreateStringConstant(value);
}

CharPtrValue::CharPtrValue() : value(nullptr)
{
}

CharPtrValue::CharPtrValue(const char* value_) : value(value_)
{
}

Value* CharPtrValue::Clone() const
{
    return new CharPtrValue(value);
}

void CharPtrValue::Read(Reader& reader)
{
    throw std::runtime_error("reading char* not supported");
}

void CharPtrValue::Write(Writer& writer)
{
    throw std::runtime_error("writing char* not supported");
}

Value* CharPtrValue::As(ValueType targetType, bool cast, const Span& span) const
{
    if (targetType != ValueType::charPtrValue)
    {
        throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType), span);
    }
    return new CharPtrValue(value);
}

Value* CharPtrValue::Deref(const Span& span) const
{
    if (value)
    {
        return new CharValue(*value);
    }
    throw Exception("tried to dereference a null char*", span);
}

Ir::Intf::Object* CharPtrValue::CreateIrObject() const
{
    throw std::runtime_error("create ir object for char* not supported");
}

void EvaluationStack::Push(Cm::Sym::Value* value)
{
    stack.push(std::unique_ptr<Cm::Sym::Value>(value));
}

Cm::Sym::Value* EvaluationStack::Pop()
{
    if (stack.empty())
    {
        throw std::runtime_error("evaluation stack is empty");
    }
    std::unique_ptr<Cm::Sym::Value> value = std::move(stack.top());
    stack.pop();
    return value.release();
}

} } // namespace Cm::Sym
