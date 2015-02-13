/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Sym/FunctionSymbol.hpp>
#include <Cm.Sym/Writer.hpp>
#include <Cm.Sym/Reader.hpp>
#include <Cm.Sym/TypeSymbol.hpp>
#include <Cm.Sym/TypeParameterSymbol.hpp>
#include <Cm.Sym/SymbolTable.hpp>
#include <Cm.Ast/Identifier.hpp>

namespace Cm { namespace Sym {

std::string FunctionSymbolFlagString(FunctionSymbolFlags flags)
{
    std::string s;
    if ((flags & FunctionSymbolFlags::constructorOrDestructorSymbol) != FunctionSymbolFlags::none)
    {
        s.append("constructor|destructor");
    }
    if ((flags & FunctionSymbolFlags::memberFunctionSymbol) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("member function");
    }
    if ((flags & FunctionSymbolFlags::external) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("external");
    }
    if ((flags & FunctionSymbolFlags::cdecl_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("cdecl");
    }
    if ((flags & FunctionSymbolFlags::virtual_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("virtual");
    }
    if ((flags & FunctionSymbolFlags::abstract_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("abstract");
    }
    if ((flags & FunctionSymbolFlags::override_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("override");
    }
    if ((flags & FunctionSymbolFlags::nothrow) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("nothrow");
    }
    if ((flags & FunctionSymbolFlags::inline_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("inline");
    }
    if ((flags & FunctionSymbolFlags::replicated) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("replicated");
    }
    if ((flags & FunctionSymbolFlags::suppressed) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("suppressed");
    }
    if ((flags & FunctionSymbolFlags::default_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("default");
    }
    if ((flags & FunctionSymbolFlags::explicit_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("explicit");
    }
    if ((flags & FunctionSymbolFlags::conversion) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("conversion");
    }
    if ((flags & FunctionSymbolFlags::templateSpecialization) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("template specialization");
    }
    return s;
}

FunctionLookup::FunctionLookup(ScopeLookup lookup_, ContainerScope* scope_) : lookup(lookup_), scope(scope_)
{
}

PersistentFunctionData::PersistentFunctionData(): bodyPos(0), bodySize(0), specifiers(), returnTypeExprNode(nullptr), groupId(nullptr), constraint(nullptr)
{
}

FunctionSymbol::FunctionSymbol(const Span& span_, const std::string& name_) : ContainerSymbol(span_, name_), returnType(nullptr), compileUnit(nullptr), flags(FunctionSymbolFlags::none), vtblIndex(-1),
    classObjectResultIrParam(nullptr)
{
}

FunctionLookupSet::FunctionLookupSet()
{
}

void FunctionLookupSet::Add(const FunctionLookup& lookup)
{
    FunctionLookup toInsert = lookup;
    for (const FunctionLookup& existingLookup : lookups)
    {
        if (toInsert.Scope() == existingLookup.Scope())
        {
            toInsert.SetLookup(toInsert.Lookup() & ~existingLookup.Lookup());
        }
    }
    if (toInsert.Lookup() != ScopeLookup::none)
    {
        lookups.push_back(toInsert);
    }
}

void FunctionSymbol::AddSymbol(Symbol* symbol)
{
    ContainerSymbol::AddSymbol(symbol);
    if (symbol->IsParameterSymbol())
    {
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameters.push_back(parameterSymbol);
    }
    else if (symbol->IsTypeParameterSymbol())
    {
        TypeParameterSymbol* typeParameterSymbol = static_cast<TypeParameterSymbol*>(symbol);
        typeParameterSymbol->SetIndex(int(typeParameters.size()));
        typeParameters.push_back(typeParameterSymbol);
    }
}

void FunctionSymbol::SetReturnType(TypeSymbol* returnType_)
{
    returnType = returnType_;
}

bool FunctionSymbol::ReturnsClassObjectByValue() const
{
    return returnType && returnType->IsClassTypeSymbol();
}

bool FunctionSymbol::IsConstructor() const
{
    return groupName == "@constructor" && !IsStatic();
}

bool FunctionSymbol::IsDefaultConstructor() const
{
    return groupName == "@constructor" && !IsStatic() && parameters.size() == 1;
}

bool FunctionSymbol::IsCopyConstructor() const
{
    if (groupName == "@constructor" && !IsStatic() && parameters.size() == 2)
    {
        Cm::Sym::TypeSymbol* firstParamType = parameters[0]->GetType();
        Cm::Sym::TypeSymbol* classType = firstParamType->GetBaseType();
        Cm::Sym::TypeSymbol* secondParamType = parameters[1]->GetType();
        if (secondParamType->IsConstReferenceType() && TypesEqual(secondParamType->GetBaseType(), classType))
        {
            return true;
        }
    }
    return false;
}

bool FunctionSymbol::IsMoveConstructor() const
{
    if (groupName == "@constructor" && !IsStatic() && parameters.size() == 2)
    {
        Cm::Sym::TypeSymbol* firstParamType = parameters[0]->GetType();
        Cm::Sym::TypeSymbol* classType = firstParamType->GetBaseType();
        Cm::Sym::TypeSymbol* secondParamType = parameters[1]->GetType();
        if (secondParamType->IsRvalueRefType() && TypesEqual(secondParamType->GetBaseType(), classType))
        {
            return true;
        }
    }
    return false;
}

bool FunctionSymbol::IsStaticConstructor() const
{
    if (groupName == "@static_constructor" && IsStatic() && parameters.empty())
    {
        return true;
    }
    return false;
}

bool FunctionSymbol::IsConvertingConstructor() const
{
    return GetFlag(FunctionSymbolFlags::conversion);
}

bool FunctionSymbol::IsExportSymbol() const 
{ 
    return IsFunctionTemplateSpecialization() ? false : ContainerSymbol::IsExportSymbol(); 
}

void FunctionSymbol::SetConvertingConstructor()
{
    SetFlag(FunctionSymbolFlags::conversion);
}

bool FunctionSymbol::CheckIfConvertingConstructor() const
{
    return IsConstructor() && parameters.size() == 2 && !IsExplicit() && !IsStatic() && !IsCopyConstructor() && !IsMoveConstructor();
}

void FunctionSymbol::SetUsingNodes(const std::vector<Cm::Ast::Node*>& usingNodes_)
{
    if (!persistentFunctionData)
    {
        persistentFunctionData.reset(new PersistentFunctionData());
        for (Cm::Ast::Node* usingNode : usingNodes_)
        {
            Cm::Ast::CloneContext cloneContext;
            persistentFunctionData->usingNodes.Add(usingNode->Clone(cloneContext));
        }
    }
}

const Cm::Ast::NodeList& FunctionSymbol::GetUsingNodes() const
{
    return persistentFunctionData->usingNodes;
}

bool FunctionSymbol::IsCopyAssignment() const
{
    if (groupName == "operator=" && parameters.size() == 2)
    {
        Cm::Sym::TypeSymbol* firstParamType = parameters[0]->GetType();
        Cm::Sym::TypeSymbol* classType = firstParamType->GetBaseType();
        Cm::Sym::TypeSymbol* secondParamType = parameters[1]->GetType();
        if (secondParamType->IsConstReferenceType() && TypesEqual(secondParamType->GetBaseType(), classType))
        {
            return true;
        }
    }
    return false;
}

bool FunctionSymbol::IsMoveAssignment() const
{
    if (groupName == "operator=" && parameters.size() == 2)
    {
        Cm::Sym::TypeSymbol* firstParamType = parameters[0]->GetType();
        Cm::Sym::TypeSymbol* classType = firstParamType->GetBaseType();
        Cm::Sym::TypeSymbol* secondParamType = parameters[1]->GetType();
        if (secondParamType->IsRvalueRefType() && TypesEqual(secondParamType->GetBaseType(), classType))
        {
            return true;
        }
    }
    return false;
}

bool FunctionSymbol::IsDestructor() const
{
    if (groupName == "@destructor" && parameters.size() == 1)
    {
        return true;
    }
    return false;
}

void FunctionSymbol::Write(Writer& writer)
{
    ContainerSymbol::Write(writer);
    writer.GetBinaryWriter().Write(uint16_t(flags));
    writer.GetBinaryWriter().Write(groupName);
    writer.GetBinaryWriter().Write(vtblIndex);
    if (IsFunctionTemplate())
    {
        SymbolTable* symbolTable = writer.GetSymbolTable();
        Cm::Ast::Node* node = symbolTable->GetNode(this);
        if (!node)
        {
            throw std::runtime_error("write: function node not found from symbol table");
        }
        if (node->IsFunctionNode())
        {
            Cm::Ast::FunctionNode* functionNode = static_cast<Cm::Ast::FunctionNode*>(node);
            bool hasReturnType = functionNode->ReturnTypeExpr();
            writer.GetBinaryWriter().Write(hasReturnType);
            if (hasReturnType)
            {
                writer.GetAstWriter().Write(functionNode->ReturnTypeExpr());
            }
            if (!functionNode->Body())
            {
                throw std::runtime_error("write: function node has no body");
            }
            writer.GetAstWriter().Write(functionNode->GetSpecifiers());
            writer.GetAstWriter().Write(functionNode->GroupId());
            bool hasConstraint = functionNode->Constraint() != nullptr;
            writer.GetBinaryWriter().Write(hasConstraint);
            if (hasConstraint)
            {
                writer.GetAstWriter().Write(functionNode->Constraint());
            }
            if (persistentFunctionData)
            {
                writer.GetBinaryWriter().Write(true);
                persistentFunctionData->usingNodes.Write(writer.GetAstWriter());
            }
            else
            {
                writer.GetBinaryWriter().Write(false);
            }
            uint64_t sizePos = writer.GetBinaryWriter().Pos();
            writer.GetBinaryWriter().Write(uint64_t(0));
            uint64_t bodyPos = writer.GetBinaryWriter().Pos();
            writer.GetAstWriter().Write(functionNode->Body());
            uint64_t bodyEndPos = writer.GetBinaryWriter().Pos();
            uint64_t bodySize = bodyEndPos - bodyPos;
            writer.GetBinaryWriter().Seek(sizePos);
            writer.GetBinaryWriter().Write(bodySize);
            writer.GetBinaryWriter().Seek(bodyEndPos);
        }
        else
        {
            throw std::runtime_error("write: not function node");
        }
    }
    else
    {
        bool hasReturnType = returnType != nullptr;
        writer.GetBinaryWriter().Write(hasReturnType);
        if (hasReturnType)
        {
            writer.Write(returnType->Id());
        }
    }
}

void FunctionSymbol::Read(Reader& reader)
{
    ContainerSymbol::Read(reader);
    flags = FunctionSymbolFlags(reader.GetBinaryReader().ReadUShort());
    groupName = reader.GetBinaryReader().ReadString();
    vtblIndex = reader.GetBinaryReader().ReadShort();
    if (IsFunctionTemplate())
    {
        persistentFunctionData.reset(new PersistentFunctionData());
        bool hasReturnType = reader.GetBinaryReader().ReadBool();
        if (hasReturnType)
        {
            persistentFunctionData->returnTypeExprNode.reset(reader.GetAstReader().ReadNode());
        }
        persistentFunctionData->specifiers = reader.GetAstReader().ReadSpecifiers();
        persistentFunctionData->groupId.reset(reader.GetAstReader().ReadFunctionGroupIdNode());
        bool hasConstraint = reader.GetBinaryReader().ReadBool();
        if (hasConstraint)
        {
            persistentFunctionData->constraint.reset(reader.GetAstReader().ReadWhereConstraintNode());
        }
        bool hasUsingNodes = reader.GetBinaryReader().ReadBool();
        if (hasUsingNodes)
        {
            persistentFunctionData->usingNodes.Read(reader.GetAstReader());
        }
        persistentFunctionData->bodySize = reader.GetBinaryReader().ReadULong();
        persistentFunctionData->bodyPos = reader.GetBinaryReader().GetPos();
        persistentFunctionData->cmlFilePath = reader.GetBinaryReader().FileName();
        reader.GetBinaryReader().Skip(persistentFunctionData->bodySize);
    }
    else
    {
        bool hasReturnType = reader.GetBinaryReader().ReadBool();
        if (hasReturnType)
        {
            reader.FetchTypeFor(this, 0);
        }
    }
}

void FunctionSymbol::SetType(TypeSymbol* type_, int index)
{
    SetReturnType(type_);
}

void FunctionSymbol::ComputeName()
{
    std::string s;
    s.append(groupName);
    if (!typeArguments.empty())
    {
        s.append(1, '<');
        bool first = true;
        for (Cm::Sym::TypeSymbol* typeArgument : typeArguments)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                s.append(", ");
            }
            s.append(typeArgument->FullName());
        }
        s.append(1, '>');
    }
    s.append(1, '(');
    bool first = true;
    for (ParameterSymbol* parameter : parameters)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            s.append(", ");
        }
        TypeSymbol* paramType = parameter->GetType();
        s.append(paramType->FullName());
    }
    s.append(1, ')');
    SetName(s);
}

TypeSymbol* FunctionSymbol::GetTargetType() const
{
    return parameters[0]->GetType()->GetBaseType();
}

void FunctionSymbol::CollectExportedDerivedTypes(std::vector<TypeSymbol*>& exportedDerivedTypes) 
{
    if (returnType)
    {
        if (returnType->IsDerivedTypeSymbol())
        {
            returnType->CollectExportedDerivedTypes(exportedDerivedTypes);
        }
    }
    for (ParameterSymbol* parameter : parameters)
    {
        parameter->CollectExportedDerivedTypes(exportedDerivedTypes);
    }
}

void FunctionSymbol::Dump(CodeFormatter& formatter)
{
    std::string s = SymbolFlagStr(Flags(), DeclaredAccess());
    if (!s.empty())
    {
        s.append(1, ' ');
    }
    s.append(FunctionSymbolFlagString(flags));
    formatter.Write(s);
    if (!s.empty())
    {
        formatter.Write(" ");
    }
    if (returnType)
    {
        formatter.Write(returnType->FullName() + " ");
    }
    formatter.Write(groupName);
    if (!typeParameters.empty())
    {
        std::string t = "<";
        bool first = true;
        for (TypeParameterSymbol* typeParam : typeParameters)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                t.append(", ");
            }
            t.append(typeParam->Name());
        }
        t.append(">");
        formatter.Write(t);
    }
    std::string p = "(";
    bool first = true;
    for (ParameterSymbol* param : parameters)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            p.append(", ");
        }
        if (param->GetType())
        {
            p.append(param->GetType()->FullName() + " ");
        }
        p.append(param->Name());
    }
    p.append(")");
    formatter.Write(p);
    formatter.WriteLine();
}

FileScope* FunctionSymbol::GetFileScope(ContainerScope* containerScope)
{
    if (!persistentFunctionData->functionFileScope)
    {
        persistentFunctionData->functionFileScope.reset(new FileScope());
        for (const std::unique_ptr<Cm::Ast::Node>& usingNode : persistentFunctionData->usingNodes)
        {
            Cm::Ast::NodeType nodeType = usingNode->GetNodeType();
            switch (nodeType)
            {
                case Cm::Ast::NodeType::aliasNode:
                {
                    Cm::Ast::AliasNode* aliasNode = static_cast<Cm::Ast::AliasNode*>(usingNode.get());
                    persistentFunctionData->functionFileScope->InstallAlias(containerScope, aliasNode);
                    break;
                }
                case Cm::Ast::NodeType::namespaceImportNode:
                {
                    Cm::Ast::NamespaceImportNode* namespaceImportNode = static_cast<Cm::Ast::NamespaceImportNode*>(usingNode.get());
                    persistentFunctionData->functionFileScope->InstallNamespaceImport(containerScope, namespaceImportNode);
                    break;
                }
            }
        }
    }
    return persistentFunctionData->functionFileScope.get();
}

} } // namespace Cm::Sym