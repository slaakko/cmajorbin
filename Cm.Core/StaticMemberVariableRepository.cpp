/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Core/StaticMemberVariableRepository.hpp>
#include <Cm.Core/Exception.hpp>
#include <Cm.Sym/ClassTypeSymbol.hpp>
#include <Cm.Sym/NameMangling.hpp>
#include <Cm.IrIntf/Rep.hpp>

namespace Cm { namespace Core {

void StaticMemberVariableRepository::Add(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol)
{
    std::string assemblyName = Cm::Sym::MakeAssemblyName(staticMemberVariableSymbol->Class()->FullName() + "." + staticMemberVariableSymbol->Name());
    Ir::Intf::Object* irObject = Cm::IrIntf::CreateGlobal(assemblyName, Cm::IrIntf::Pointer(staticMemberVariableSymbol->GetType()->GetIrType(), 1));
    StaticMemberVariableMapIt i = staticMemberVariableMap.find(staticMemberVariableSymbol);
    if (i != staticMemberVariableMap.end())
    {
        throw Cm::Core::Exception("static member variable '" + staticMemberVariableSymbol->FullName() + "' already added to repository", staticMemberVariableSymbol->GetSpan());
    }
    staticMemberVariableMap[staticMemberVariableSymbol] = irObject;
    ownedIrObjects.push_back(std::unique_ptr<Ir::Intf::Object>(irObject));
    if (staticMemberVariableSymbol->GetType()->IsClassTypeSymbol() && static_cast<Cm::Sym::ClassTypeSymbol*>(staticMemberVariableSymbol->GetType())->Destructor())
    {
        Ir::Intf::Object* destructionNode = Cm::IrIntf::CreateGlobal(Cm::IrIntf::MakeDestructionNodeName(assemblyName),
            Cm::IrIntf::Pointer(Cm::IrIntf::CreateTypeName(Cm::IrIntf::GetDestructionNodeTypeName(), false), 1));
        ownedIrObjects.push_back(std::unique_ptr<Ir::Intf::Object>(destructionNode));
        destructionNodeMap[staticMemberVariableSymbol] = destructionNode;
    }
}

Ir::Intf::Object* StaticMemberVariableRepository::GetStaticMemberVariableIrObject(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol) const
{
    StaticMemberVariableMapIt i = staticMemberVariableMap.find(staticMemberVariableSymbol);
    if (i != staticMemberVariableMap.end())
    {
        return i->second;
    }
    throw Cm::Core::Exception("static member variable '" + staticMemberVariableSymbol->FullName() + "' not found in repository", staticMemberVariableSymbol->GetSpan());
}

Ir::Intf::Object* StaticMemberVariableRepository::GetDestructionNode(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol) const
{
    StaticMemberVariableMapIt i = destructionNodeMap.find(staticMemberVariableSymbol);
    if (i != destructionNodeMap.end())
    {
        return i->second;
    }
    throw Cm::Core::Exception("destruction node for '" + staticMemberVariableSymbol->FullName() + "' not found in repository", staticMemberVariableSymbol->GetSpan());
}

void StaticMemberVariableRepository::Write(Cm::Util::CodeFormatter& codeFormatter)
{
    for (const std::pair<Cm::Sym::MemberVariableSymbol*, Ir::Intf::Object*>& p : staticMemberVariableMap)
    {
        Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol = p.first;
        Ir::Intf::Object* irObject = p.second;
        std::string declaration = irObject->Name();
        declaration.append(" = global ").append(staticMemberVariableSymbol->GetType()->GetIrType()->Name());
        Ir::Intf::Object* defaultValue = staticMemberVariableSymbol->GetType()->GetDefaultIrValue();
        if (defaultValue)
        {
            declaration.append(1, ' ').append(defaultValue->Name());
        }
        else
        {
            declaration.append(" zeroinitializer");
        }
        codeFormatter.WriteLine(declaration);
    }
    for (const std::pair<Cm::Sym::MemberVariableSymbol*, Ir::Intf::Object*>& p : destructionNodeMap)
    {
        Ir::Intf::Object* destructionNode = p.second;
        std::string destructionNodeDeclaration(destructionNode->Name());
        destructionNodeDeclaration.append(" = global %" + Cm::IrIntf::GetDestructionNodeTypeName() + " zeroinitializer");
        codeFormatter.WriteLine(destructionNodeDeclaration);
    }
}

} } // namespace Cm::Core