/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Bind/LocalVariable.hpp>
#include <Cm.Core/Exception.hpp>
#include <Cm.Bind/TypeResolver.hpp>
#include <Cm.Sym/LocalVariableSymbol.hpp>
#include <Cm.Ast/Identifier.hpp>

namespace Cm { namespace Bind {

Cm::Sym::LocalVariableSymbol* BindLocalVariable(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes,
    Cm::Core::ClassTemplateRepository& classTemplateRepository, Cm::BoundTree::BoundCompileUnit& boundCompileUnit, Cm::Ast::ConstructionStatementNode* constructionStatementNode)
{
    Cm::Sym::Symbol* symbol = containerScope->Lookup(constructionStatementNode->Id()->Str(), Cm::Sym::SymbolTypeSetId::lookupLocalVariable);
    if (symbol)
    {
        if (symbol->IsLocalVariableSymbol())
        {
            Cm::Sym::LocalVariableSymbol* localVariableSymbol = static_cast<Cm::Sym::LocalVariableSymbol*>(symbol);
            return BindLocalVariable(symbolTable, containerScope, fileScopes, classTemplateRepository, boundCompileUnit, constructionStatementNode, localVariableSymbol);
        }
        else
        {
            throw Cm::Core::Exception("symbol '" + symbol->FullName() + "' does not denote a local variable", symbol->GetSpan());
        }
    }
    else
    {
        throw Cm::Core::Exception("local variable symbol '" + constructionStatementNode->Id()->Str() + "' not found");
    }
}

Cm::Sym::LocalVariableSymbol* BindLocalVariable(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes,
    Cm::Core::ClassTemplateRepository& classTemplateRepository, Cm::BoundTree::BoundCompileUnit& boundCompileUnit, Cm::Ast::ConstructionStatementNode* constructionStatementNode, Cm::Sym::LocalVariableSymbol* localVariableSymbol)
{
    if (localVariableSymbol->Bound())
    {
        return localVariableSymbol;
    }
    Cm::Sym::TypeSymbol* type = ResolveType(symbolTable, containerScope, fileScopes, classTemplateRepository, boundCompileUnit, constructionStatementNode->TypeExpr());
    localVariableSymbol->SetType(type);
    localVariableSymbol->SetBound();
    return localVariableSymbol;
}

} } // namespace Cm::Bind