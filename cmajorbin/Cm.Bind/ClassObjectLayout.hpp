/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_BIND_CLASS_OBJECT_LAYOUT_INCLUDED
#define CM_BIND_CLASS_OBJECT_LAYOUT_INCLUDED
#include <Cm.Bind/ExpressionBinder.hpp>
#include <Cm.Sym/ClassTypeSymbol.hpp>

namespace Cm { namespace Bind {

void GenerateClassInitStatement(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ConversionTable& conversionTable, Cm::Core::ClassConversionTable& classConversionTable,
    Cm::Core::DerivedTypeOpRepository& derivedTypeOpRepository, Cm::Core::SynthesizedClassFunRepository& synthesizedClassFunRepository, Cm::Core::StringRepository& stringRepository, 
    Cm::Core::IrClassTypeRepository& irClassTypeRepository, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::BoundTree::BoundFunction* currentFunction,
    Cm::Sym::ClassTypeSymbol* classType, Cm::Ast::ConstructorNode* constructorNode, bool& callToThisInitializerGenerated);

void GenerateInitVPtrStatement(Cm::Sym::ClassTypeSymbol* classType, Cm::BoundTree::BoundFunction* boundFunction);

void GenerateMemberVariableInitStatements(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ConversionTable& conversionTable, Cm::Core::ClassConversionTable& classConversionTable,
    Cm::Core::DerivedTypeOpRepository& derivedTypeOpRepository, Cm::Core::SynthesizedClassFunRepository& synthesizedClassFunRepository, Cm::Core::StringRepository& stringRepository, 
    Cm::Core::IrClassTypeRepository& irClassTypeRepository, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::BoundTree::BoundFunction* currentFunction,
    Cm::Sym::ClassTypeSymbol* classType, Cm::Ast::ConstructorNode* constructorNode);

void GenerateMemberVariableDestructionStatements(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ConversionTable& conversionTable, Cm::Core::ClassConversionTable& classConversionTable,
    Cm::Core::DerivedTypeOpRepository& derivedTypeOpRepository, Cm::Core::SynthesizedClassFunRepository& synthesizedClassFunRepository, Cm::Core::StringRepository& stringRepository,
    Cm::Core::IrClassTypeRepository& irClassTypeRepository, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::BoundTree::BoundFunction* currentFunction,
    Cm::Sym::ClassTypeSymbol* classType, Cm::Ast::DestructorNode* destructorNode);

void GenerateBaseClassDestructionStatement(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ConversionTable& conversionTable, Cm::Core::ClassConversionTable& classConversionTable,
    Cm::Core::DerivedTypeOpRepository& derivedTypeOpRepository, Cm::Core::SynthesizedClassFunRepository& synthesizedClassFunRepository, Cm::Core::StringRepository& stringRepository,
    Cm::Core::IrClassTypeRepository& irClassTypeRepository, Cm::Sym::ContainerScope* containerScope, Cm::Sym::FileScope* fileScope, Cm::BoundTree::BoundFunction* currentFunction,
    Cm::Sym::ClassTypeSymbol* classType, Cm::Ast::DestructorNode* destructorNode);

} } // namespace Cm::Bind

#endif // CM_BIND_CLASS_OBJECT_LAYOUT_INCLUDED