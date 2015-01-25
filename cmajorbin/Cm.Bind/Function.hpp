/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_BIND_FUNCTION_INCLUDED
#define CM_BIND_FUNCTION_INCLUDED
#include <Cm.BoundTree/BoundFunction.hpp>
#include <Cm.Sym/SymbolTable.hpp>
#include <Cm.Ast/Function.hpp>

namespace Cm { namespace Bind {

Cm::Sym::FunctionSymbol* BindFunction(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::Ast::FunctionNode* functionNode, 
    Cm::Sym::ClassTypeSymbol* currentClass);

void CompleteBindFunction(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::Ast::FunctionNode* functionNode, 
    Cm::Sym::FunctionSymbol* functionSymbol, Cm::Sym::ClassTypeSymbol* currentClass);

void CheckFunctionReturnPaths(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::Sym::FunctionSymbol* functionSymbol, 
    Cm::Ast::FunctionNode* functionNode);

void CheckFunctionAccessLevels(Cm::Sym::FunctionSymbol* functionSymbol);

} } // namespace Cm::Bind

#endif // CM_BIND_FUNCTION_INCLUDED
