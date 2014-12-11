/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_SYM_TEMPLATE_TYPE_SYMBOL_INCLUDED
#define CM_SYM_TEMPLATE_TYPE_SYMBOL_INCLUDED
#include <Cm.Sym/TypeSymbol.hpp>

namespace Cm { namespace Sym {

class TemplateTypeSymbol : public TypeSymbol
{
public:
    TemplateTypeSymbol(const Span& span_, const std::string& name_);
    SymbolType GetSymbolType() const override { return SymbolType::templateTypeSymbol; }
    bool IsExportSymbol() const override { return Source() == SymbolSource::project; }
    void Write(Writer& writer) override;
    void Read(Reader& reader) override;
    void SetSubjectType(TypeSymbol* subjectType_);
    void AddTypeArgument(TypeSymbol* typeArgument);
    void SetType(TypeSymbol* type, int index) override;
private:
    TypeSymbol* subjectType;
    std::vector<TypeSymbol*> typeArguments;
};

} } // namespace Cm::Sym

#endif // CM_SYM_TEMPLATE_TYPE_SYMBOL_INCLUDED
