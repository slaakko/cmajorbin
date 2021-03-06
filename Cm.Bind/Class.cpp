/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Bind/Class.hpp>
#include <Cm.Bind/Interface.hpp>
#include <Cm.Bind/Access.hpp>
#include <Cm.Bind/TypeResolver.hpp>
#include <Cm.Bind/MemberVariable.hpp>
#include <Cm.Core/Exception.hpp>
#include <Cm.Sym/ClassTypeSymbol.hpp>
#include <Cm.Sym/InterfaceTypeSymbol.hpp>
#include <Cm.Sym/TemplateTypeSymbol.hpp>
#include <Cm.Sym/TypeSymbol.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.IrIntf/Rep.hpp>

namespace Cm { namespace Bind {

Cm::Sym::ClassTypeSymbol* BindClass(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes,
    Cm::Core::ClassTemplateRepository& classTemplateRepository, Cm::BoundTree::BoundCompileUnit& boundCompileUnit, Cm::Ast::ClassNode* classNode)
{
    Cm::Sym::Symbol* symbol = containerScope->Lookup(classNode->Id()->Str(), Cm::Sym::ScopeLookup::this_and_base_and_parent, Cm::Sym::SymbolTypeSetId::lookupClassSymbols);
    if (symbol)
    {
        if (symbol->IsClassTypeSymbol())
        {
            Cm::Sym::ClassTypeSymbol* classTypeSymbol = static_cast<Cm::Sym::ClassTypeSymbol*>(symbol);
            BindClass(symbolTable, containerScope, fileScopes, classTemplateRepository, boundCompileUnit, classNode, classTypeSymbol);
            return classTypeSymbol;
        }
        else
        {
            throw Cm::Core::Exception("symbol '" + symbol->FullName() + "' does not denote a class", symbol->GetSpan());
        }
    }
    else
    {
        throw Cm::Core::Exception("class symbol '" + classNode->Id()->Str() + "' not found");
    }
}

void BindClass(Cm::Sym::SymbolTable& symbolTable, Cm::Sym::ContainerScope* containerScope, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes,
    Cm::Core::ClassTemplateRepository& classTemplateRepository, Cm::BoundTree::BoundCompileUnit& boundCompileUnit, Cm::Ast::ClassNode* classNode, Cm::Sym::ClassTypeSymbol* classTypeSymbol)
{
    if (classTypeSymbol->Bound()) return;
    if (classNode->GetCompileUnit())
    {
        classTypeSymbol->SetSourceFilePath(classNode->GetCompileUnit()->FilePath());
    }
    Cm::Ast::Specifiers specifiers = classNode->GetSpecifiers();
    bool isClassMember = classNode->Parent()->IsClassNode();
    SetAccess(classTypeSymbol, specifiers, isClassMember);
    if ((specifiers & Cm::Ast::Specifiers::static_) != Cm::Ast::Specifiers::none)
    {
        classTypeSymbol->SetStatic();
    }
    if ((specifiers & Cm::Ast::Specifiers::abstract_) != Cm::Ast::Specifiers::none)
    {
        classTypeSymbol->SetAbstract();
    }
    if ((specifiers & Cm::Ast::Specifiers::virtual_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be virtual", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::override_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be override", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::explicit_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be explicit", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::external) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be external", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::suppress) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be suppressed", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::default_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be default", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::inline_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be inline", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::constexpr_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be constexpr", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::cdecl_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be cdecl", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::nothrow_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be nothrow", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::throw_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be throw", classTypeSymbol->GetSpan());
    }
    if ((specifiers & Cm::Ast::Specifiers::new_) != Cm::Ast::Specifiers::none)
    {
        throw Cm::Core::Exception("class cannnot be new", classTypeSymbol->GetSpan());
    }
    if (classNode->TemplateParameters().Count() > 0)
    {
        classTypeSymbol->SetBound();
        return;
    }
    for (const std::unique_ptr<Cm::Ast::Node>& baseClassOrImplIntf : classNode->BaseClassOrImplIntfTypeExprs())
    {
        Cm::Sym::TypeSymbol* type = ResolveType(symbolTable, classTypeSymbol->GetContainerScope(), fileScopes, classTemplateRepository, boundCompileUnit, baseClassOrImplIntf.get());
        if (type)
        {
            if (type->IsClassTypeSymbol())
            {
                Cm::Sym::ClassTypeSymbol* baseClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(type);
                if (Cm::Sym::TypesEqual(classTypeSymbol, baseClassType))
                {
                    Cm::Core::Exception("class cannot derive from itself", classTypeSymbol->GetSpan());
                }
                else
                {
                    if (!classTypeSymbol->BaseClassSet() && classTypeSymbol->BaseClass())
                    {
                        throw Cm::Core::Exception("class cannot have more than one base class", classTypeSymbol->GetSpan(), baseClassType->GetSpan());
                    }
                    Cm::Ast::Node* node = symbolTable.GetNode(baseClassType, false);
                    if (node)
                    {
                        if (node->IsClassNode())
                        {
                            Cm::Ast::ClassNode* baseClassNode = static_cast<Cm::Ast::ClassNode*>(node);
                            Cm::Sym::ContainerScope* baseClassContainerScope = symbolTable.GetContainerScope(baseClassNode);
                            BindClass(symbolTable, baseClassContainerScope, fileScopes, classTemplateRepository, boundCompileUnit, baseClassNode, baseClassType);
                        }
                        else
                        {
                            throw std::runtime_error("class node expected");
                        }
                    }
                    if (baseClassType->Access() < classTypeSymbol->Access())
                    {
                        throw Cm::Core::Exception("base class type must be at least as accessible as the class type itself", baseClassType->GetSpan(), classTypeSymbol->GetSpan());
                    }
                    classTypeSymbol->SetBaseClass(baseClassType);
                    classTypeSymbol->GetContainerScope()->SetBase(baseClassType->GetContainerScope());
                }
            }
            else if (type->IsInterfaceTypeSymbol())
            {
                Cm::Sym::InterfaceTypeSymbol* interfaceType = static_cast<Cm::Sym::InterfaceTypeSymbol*>(type);
                Cm::Ast::Node* node = symbolTable.GetNode(interfaceType, false);
                if (node)
                {
                    if (node->IsInterfaceNode())
                    {
                        Cm::Ast::InterfaceNode* interfaceNode = static_cast<Cm::Ast::InterfaceNode*>(node);
                        Cm::Sym::ContainerScope* interfaceContainerScope = symbolTable.GetContainerScope(interfaceNode);
                        BindInterface(symbolTable, interfaceContainerScope, fileScopes, interfaceNode);
                    }
                    else
                    {
                        throw std::runtime_error("interface node expected");
                    }
                }
                if (!classTypeSymbol->ImplementedInterfacesSet())
                {
                    for (Cm::Sym::InterfaceTypeSymbol* implementedInterface : classTypeSymbol->ImplementedInterfaces())
                    {
                        if (Cm::Sym::TypesEqual(interfaceType, implementedInterface))
                        {
                            throw Cm::Core::Exception("class already implements interface '" + implementedInterface->FullName() + "'", interfaceType->GetSpan());
                        }
                    }
                    classTypeSymbol->AddImplementedInterface(interfaceType);
                }
            }
            else
            {
                throw Cm::Core::Exception("type expression does not denote a class type or interface type", baseClassOrImplIntf->GetSpan());
            }
        }
        else
        {
            throw Cm::Core::Exception("type expression does not denote a type", baseClassOrImplIntf->GetSpan());
        }
    }
    if (!classTypeSymbol->IrTypeMade())
    {
        classTypeSymbol->SetIrType(Cm::IrIntf::CreateClassTypeName(classTypeSymbol->FullName()));
    }
    classTypeSymbol->SetBaseClassSet();
    classTypeSymbol->SetImplementedInterfacesSet();
    classTypeSymbol->SetBound();
}

void AddClassTypeToIrClassTypeRepository(Cm::Sym::ClassTypeSymbol* classType, Cm::BoundTree::BoundCompileUnit& boundCompileUnit, Cm::Sym::ContainerScope* containerScope)
{
    if (boundCompileUnit.IsPrebindCompileUnit()) return;
    if (boundCompileUnit.IrClassTypeRepository().Added(classType)) return;
    boundCompileUnit.IrClassTypeRepository().AddClassType(classType);
    if (!classType->Bound())
    {
        if (classType->IsTemplateTypeSymbol())
        {
            Cm::Sym::TemplateTypeSymbol* templateTypeSymbol = static_cast<Cm::Sym::TemplateTypeSymbol*>(classType);
            boundCompileUnit.ClassTemplateRepository().BindTemplateTypeSymbol(templateTypeSymbol, containerScope, boundCompileUnit.GetFileScopes());
            boundCompileUnit.ClassTemplateRepository().InstantiateVirtualFunctionsFor(containerScope, templateTypeSymbol);
        }
    }
    if (classType->BaseClass())
    {
        AddClassTypeToIrClassTypeRepository(classType->BaseClass(), boundCompileUnit, containerScope);
    }
    for (Cm::Sym::InterfaceTypeSymbol* intf : classType->ImplementedInterfaces())
    {
        boundCompileUnit.IrInterfaceTypeRepository().Add(intf);
    }
    for (Cm::Sym::MemberVariableSymbol* memberVar : classType->MemberVariables())
    {
        Cm::Sym::TypeSymbol* memberVariableBaseType = memberVar->GetType()->GetBaseType();
        if (memberVariableBaseType->IsClassTypeSymbol())
        {
            AddClassTypeToIrClassTypeRepository(static_cast<Cm::Sym::ClassTypeSymbol*>(memberVariableBaseType), boundCompileUnit, containerScope);
        }
        else if (memberVariableBaseType->IsInterfaceTypeSymbol())
        {
            boundCompileUnit.IrInterfaceTypeRepository().Add(static_cast<Cm::Sym::InterfaceTypeSymbol*>(memberVariableBaseType));
        }
    }
    if (classType->IsVirtual())
    {
        for (Cm::Sym::FunctionSymbol* virtualFunction : classType->Vtbl())
        {
            if (virtualFunction)
            {
                Cm::Sym::TypeSymbol* returnType = virtualFunction->GetReturnType();
                if (returnType && returnType->GetBaseType()->IsClassTypeSymbol())
                {
                    Cm::Sym::ClassTypeSymbol* returnClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(returnType->GetBaseType());
                    AddClassTypeToIrClassTypeRepository(returnClassType, boundCompileUnit, containerScope);
                }
                else if (returnType && returnType->GetBaseType()->IsInterfaceTypeSymbol())
                {
                    boundCompileUnit.IrInterfaceTypeRepository().Add(static_cast<Cm::Sym::InterfaceTypeSymbol*>(returnType->GetBaseType()));
                }
                for (Cm::Sym::ParameterSymbol* parameter : virtualFunction->Parameters())
                {
                    Cm::Sym::TypeSymbol* parameterBaseType = parameter->GetType()->GetBaseType();
                    if (parameterBaseType->IsClassTypeSymbol())
                    {
                        Cm::Sym::ClassTypeSymbol* parameterClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(parameterBaseType);
                        AddClassTypeToIrClassTypeRepository(parameterClassType, boundCompileUnit, containerScope);
                    }
                    else if (parameterBaseType->IsInterfaceTypeSymbol())
                    {
                        boundCompileUnit.IrInterfaceTypeRepository().Add(static_cast<Cm::Sym::InterfaceTypeSymbol*>(parameterBaseType));
                    }
                }
            }
        }
    }
}

} } // namespace Cm::Bind

