/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Sym/EnumSymbol.hpp>
#include <Cm.Sym/BasicTypeSymbol.hpp>
#include <Cm.Sym/Writer.hpp>
#include <Cm.Sym/Reader.hpp>
#include <Cm.Ast/Identifier.hpp>

namespace Cm { namespace Sym {

EnumTypeSymbol::EnumTypeSymbol(const Span& span_, const std::string& name_) : TypeSymbol(span_, name_), underlyingType(nullptr)
{
}

void EnumTypeSymbol::Write(Writer& writer)
{
    TypeSymbol::Write(writer);
    writer.Write(underlyingType->Id());
}

void EnumTypeSymbol::Read(Reader& reader)
{
    TypeSymbol::Read(reader);
    reader.FetchTypeFor(this, 0);
}

void EnumTypeSymbol::SetUnderlyingType(TypeSymbol* underlyingType_)
{
    underlyingType = underlyingType_;
}

EnumConstantSymbol::EnumConstantSymbol(const Span& span_, const std::string& name_) : Symbol(span_, name_), evaluating(false)
{
}

void EnumConstantSymbol::Write(Writer& writer)
{
    Symbol::Write(writer);
    writer.Write(value.get());
}

void EnumConstantSymbol::Read(Reader& reader)
{
    Symbol::Read(reader);
    value.reset(reader.ReadValue());
}

void EnumConstantSymbol::SetValue(Value* value_)
{
    value.reset(value_);
}

} } // namespace Cm::Sym