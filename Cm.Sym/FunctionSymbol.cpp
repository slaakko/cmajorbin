/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
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
#include <Cm.Sym/GlobalFlags.hpp>
#include <Cm.Sym/NameMangling.hpp>
#include <Cm.Sym/ClassTypeSymbol.hpp>
#include <Cm.Sym/TemplateTypeSymbol.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.Ast/Clone.hpp>
#include <iostream>
#include <stdexcept>

namespace Cm { namespace Sym {

std::string FunctionSymbolFlagString(FunctionSymbolFlags flags)
{
    std::string s;
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
    if ((flags & FunctionSymbolFlags::constexpr_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("constexpr");
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
    if ((flags & FunctionSymbolFlags::new_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("new");
    }
    if ((flags & FunctionSymbolFlags::intrinsic) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("intrinsic");
    }
    return s;
}

FunctionLookup::FunctionLookup(ScopeLookup lookup_, ContainerScope* scope_) : lookup(lookup_), scope(scope_)
{
}

PersistentFunctionData::PersistentFunctionData(): functionPos(0), functionSize(0)
{
}

FunctionSymbol::FunctionSymbol(const Span& span_, const std::string& name_) : ContainerSymbol(span_, name_), returnType(nullptr), compileUnit(nullptr), flags(FunctionSymbolFlags::none), vtblIndex(-1),
    itblIndex(-1), classOrInterfaceObjectResultIrParam(nullptr), mutexId(-1), overriddenFunction(nullptr), functionTemplate(nullptr), entrySymbol(nullptr), returnValueSymbol(nullptr)
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
    else if (symbol->IsEntrySymbol())
    {
        entrySymbol = static_cast<EntrySymbol*>(symbol);
    }
    else if (symbol->IsReturnValueSymbol())
    {
        returnValueSymbol = static_cast<ReturnValueSymbol*>(symbol);
    }
}

void FunctionSymbol::SetReturnType(TypeSymbol* returnType_)
{
    returnType = returnType_;
}

bool FunctionSymbol::ReturnsClassOrInterfaceObjectByValue() const
{
    return returnType && (returnType->IsClassTypeSymbol() || returnType->IsInterfaceTypeSymbol());
}

ParameterSymbol* FunctionSymbol::ThisParameter() const
{
    return parameters[0];
}

EntrySymbol* FunctionSymbol::Entry() const
{
    return entrySymbol;
}

ReturnValueSymbol* FunctionSymbol::ReturnValue() const
{
    return returnValueSymbol;
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
    return IsConstructor() && GetFlag(FunctionSymbolFlags::conversion);
}

bool FunctionSymbol::IsConversionFunction() const
{
    return !IsConstructor() && GetFlag(FunctionSymbolFlags::conversion);
}

bool FunctionSymbol::IsExportSymbol() const 
{
    if (IsIntrinsic()) return false;
    if (IsFunctionTemplateSpecialization()) return false;
    if (Parent()->IsClassTemplateSymbol()) return false;
    if (Parent()->IsTemplateTypeSymbol()) return false;
    if (IsReplica()) return false;
    if (Source() == SymbolSource::project && GetGlobalFlag(GlobalFlags::optimize) && IsInline()) return true;
    if (Source() == SymbolSource::project && IsConstExpr()) return true;
    return ContainerSymbol::IsExportSymbol();
}

void FunctionSymbol::SetConvertingConstructor()
{
    SetFlag(FunctionSymbolFlags::conversion);
}

void FunctionSymbol::SetConversionFunction()
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
    if (!persistentFunctionData)
    {
        throw std::runtime_error("no persistent function data");
    }
    return persistentFunctionData->usingNodes;
}

void FunctionSymbol::SetConstraint(Cm::Ast::WhereConstraintNode* constraint)
{
    if (!persistentFunctionData)
    {
        persistentFunctionData.reset(new PersistentFunctionData());
    }
    persistentFunctionData->constraint.reset(constraint);
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

bool FunctionSymbol::IsClassOpEqual() const
{
    if (groupName == "operator==" && parameters.size() == 2)
    {
        Cm::Sym::TypeSymbol* firstParamType = parameters[0]->GetType();
        Cm::Sym::TypeSymbol* classType = firstParamType->GetBaseType();
        if (classType->IsClassTypeSymbol())
        {
            Cm::Sym::TypeSymbol* secondParamType = parameters[1]->GetType();
            if (TypesEqual(secondParamType->GetBaseType(), classType))
            {
                return true;
            }
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
    bool justSymbol = false;
    if (IsFunctionTemplateSpecialization() || IsMemberOfTemplateType())
    {
        justSymbol = true;
        SetJustSymbol();
    }
    ContainerSymbol::Write(writer);
    writer.GetBinaryWriter().Write(uint32_t(flags));
    writer.GetBinaryWriter().Write(groupName);
    writer.GetBinaryWriter().Write(vtblIndex);
    writer.GetBinaryWriter().Write(itblIndex);
    if ((IsFunctionTemplate() || IsConstExpr() || (IsInline() && GetGlobalFlag(GlobalFlags::optimize) && !IsMemberOfClassTemplate())) && !justSymbol)
    {
        if (persistentFunctionData)
        {
            writer.GetBinaryWriter().Write(true);
            persistentFunctionData->usingNodes.Write(writer.GetAstWriter());
        }
        else
        {
            writer.GetBinaryWriter().Write(false);
        }
        SymbolTable* symbolTable = writer.GetSymbolTable();
        Cm::Ast::Node* node = symbolTable->GetNode(this);
        if (!node)
        {
            throw std::runtime_error("write: function node not found from symbol table");
        }
        if (node->IsFunctionNode())
        {
            Cm::Ast::FunctionNode* functionNode = static_cast<Cm::Ast::FunctionNode*>(node);
            Cm::Ast::WhereConstraintNode* constraint = functionNode->Constraint();
            bool hasConstraint = constraint != nullptr;
            writer.GetBinaryWriter().Write(hasConstraint);
            if (hasConstraint)
            {
                writer.GetAstWriter().Write(constraint);
            }
            uint64_t sizePos = writer.GetBinaryWriter().Pos();
            writer.GetBinaryWriter().Write(uint64_t(0));
            uint64_t functionPos = writer.GetBinaryWriter().Pos();
            writer.GetAstWriter().Write(functionNode);
            uint64_t functionEndPos = writer.GetBinaryWriter().Pos();
            uint64_t functionSize = functionEndPos - functionPos;
            writer.GetBinaryWriter().Seek(sizePos);
            writer.GetBinaryWriter().Write(functionSize);
            writer.GetBinaryWriter().Seek(functionEndPos);
            bool hasReturnType = returnType != nullptr;
            writer.GetBinaryWriter().Write(hasReturnType);
            if (hasReturnType)
            {
                writer.Write(returnType->Id());
            }
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
    int32_t n = int32_t(typeArguments.size());
    writer.GetBinaryWriter().Write(n);
    for (int32_t i = 0; i < n; ++i)
    {
        TypeSymbol* typeArgument = typeArguments[i];
        writer.Write(typeArgument->Id());
    }
}

void FunctionSymbol::Read(Reader& reader)
{
    ContainerSymbol::Read(reader);
    flags = FunctionSymbolFlags(reader.GetBinaryReader().ReadUInt());
    groupName = reader.GetBinaryReader().ReadString();
    vtblIndex = reader.GetBinaryReader().ReadShort();
    itblIndex = reader.GetBinaryReader().ReadShort();
    bool justSymbol = JustSymbol();
    bool isConstExpr = IsConstExpr();
    if ((IsFunctionTemplate() || IsConstExpr() || (IsInline() && GetGlobalFlag(GlobalFlags::optimize) && !IsMemberOfClassTemplate())) && !justSymbol)
    {
        persistentFunctionData.reset(new PersistentFunctionData());
        persistentFunctionData->cmlFilePath = reader.GetBinaryReader().FileName();
        bool hasUsingNodes = reader.GetBinaryReader().ReadBool();
        if (hasUsingNodes)
        {
            persistentFunctionData->usingNodes.Read(reader.GetAstReader());
        }
        bool hasConstaint = reader.GetBinaryReader().ReadBool();
        if (hasConstaint)
        {
            persistentFunctionData->constraint.reset(reader.GetAstReader().ReadWhereConstraintNode());
        }
        persistentFunctionData->functionSize = reader.GetBinaryReader().ReadULong();
        persistentFunctionData->functionPos = reader.GetBinaryReader().GetPos();
        reader.GetBinaryReader().Skip(persistentFunctionData->functionSize);
        bool hasReturnType = reader.GetBinaryReader().ReadBool();
        if (hasReturnType)
        {
            reader.FetchTypeFor(this, -1);
        }
    }
    else
    {
        bool hasReturnType = reader.GetBinaryReader().ReadBool();
        if (hasReturnType)
        {
            reader.FetchTypeFor(this, -1);
        }
    }
    int n = reader.GetBinaryReader().ReadInt();
    typeArguments.resize(n);
    for (int i = 0; i < n; ++i)
    {
        reader.FetchTypeFor(this, i);
    }
}

void FunctionSymbol::ReadFunctionNode(Cm::Sym::SymbolTable& symbolTable, int fileIndex)
{
    if (!persistentFunctionData)
    {
        throw std::runtime_error("no persistent function data");
    }
    const std::string& cmlFilePath = persistentFunctionData->cmlFilePath;
    Cm::Ser::BinaryReader binaryReader(cmlFilePath);
    binaryReader.SetPos(persistentFunctionData->functionPos);
    Cm::Ast::Reader astReader(binaryReader);
    astReader.SetReplaceFileIndex(fileIndex);
    Cm::Ast::Node* node = astReader.ReadNode();
    if (node->IsFunctionNode())
    {
        symbolTable.SetNode(this, node);
        persistentFunctionData->functionNode.reset(static_cast<Cm::Ast::FunctionNode*>(node));
    }
    else
    {
        throw std::runtime_error("not function node");
    }
}

void FunctionSymbol::FreeFunctionNode(Cm::Sym::SymbolTable& symbolTable)
{
    if (!persistentFunctionData)
    {
        throw std::runtime_error("no persistent function data");
    }
    if (persistentFunctionData->functionNode)
    {
        persistentFunctionData->functionNode.reset();
        symbolTable.SetNode(this, nullptr);
    }
}

void FunctionSymbol::SetType(TypeSymbol* type_, int index)
{
    if (index == -1)
    {
        SetReturnType(type_);
    }
    else
    {
        if (index >= int(typeArguments.size()))
        {
            throw std::runtime_error("invalid type index");
        }
        typeArguments[index] = type_;
    }
}

void FunctionSymbol::ComputeName()
{
    std::string s;
    if (IsConversionFunction())
    {
        groupName = "operator_" + GetReturnType()->FullName();
    }
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
    bool isConst = false;
    s.append(1, '(');
    bool first = true;
    for (ParameterSymbol* parameter : parameters)
    {
        if (first)
        {
            first = false;
            if (IsMemberFunctionSymbol())
            {
                if (parameter->GetType()->IsConstType())
                {
                    isConst = true;
                }
            }
        }
        else
        {
            s.append(", ");
        }
        TypeSymbol* paramType = parameter->GetType();
        s.append(paramType->FullName());
    }
    s.append(1, ')');
    if (isConst)
    {
        s.append(" const");
    }
    SetName(s);
}

bool FunctionSymbol::IsConst() const
{
    if (IsMemberFunctionSymbol() && !parameters.empty())
    {
        if (parameters[0]->GetType()->IsConstType())
        {
            return true;
        }
    }
    return false;
}

std::string FunctionSymbol::FullDocId() const
{
    std::string fullDocId;
    Symbol* p = Parent();
    if (p)
    {
        fullDocId = p->FullDocId();
    }
    if (!fullDocId.empty())
    {
        fullDocId.append(1, '.');
    }
    fullDocId.append(Cm::Sym::MakeGroupDocId(groupName));
    if (!typeParameters.empty())
    {
        for (TypeParameterSymbol* typeParam : typeParameters)
        {
            fullDocId.append(1, '.').append(typeParam->Name());
        }
    }
    if (!typeArguments.empty())
    {
        for (TypeSymbol* typeArgument : typeArguments)
        {
            fullDocId.append(1, '.').append(typeArgument->Name());
        }
    }
    if (!IsFunctionTemplate())
    {
        for (ParameterSymbol* parameter : parameters)
        {
            fullDocId.append(1, '.').append(parameter->GetType()->FullDocId());
        }
    }
    if (!constraintDocId.empty())
    {
        fullDocId.append(1, '.').append(constraintDocId);
    }
    return fullDocId;
}

std::string FunctionSymbol::Syntax() const
{
    std::string syntax = SymbolFlagStr(Flags(), DeclaredAccess(), true);
    if (!syntax.empty())
    {
        syntax.append(1, ' ');
    }
    if (IsAbstract())
    {
        syntax.append("abstract ");
    }
    if (returnType)
    {
        syntax.append(returnType->FullName()).append(" ");
    }
    if (IsIntrinsic())
    {
        syntax.append(groupName);
        if (typeParameters.size() > 0)
        {
            syntax.append(1, '<');
            bool first = true;
            for (TypeParameterSymbol* typeParam : typeParameters)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    syntax.append(", ");
                }
                syntax.append(typeParam->Name());
            }
            syntax.append(1, '>');
        }
        syntax.append("(");
        int n = int(Parameters().size());
        for (int i = 0; i < n; ++i)
        {
            if (i > 0)
            {
                syntax.append(", ");
            }
            ParameterSymbol* param = Parameters()[i];
            syntax.append(param->GetType()->FullName()).append(" ").append(param->Name());
        }
        syntax.append(")");
    }
    else
    {
        int startParamIndex = 0;
        if (IsMemberFunctionSymbol())
        {
            if (!IsStatic())
            {
                startParamIndex = 1;
            }
            Symbol* parent = Parent();
            ClassTypeSymbol* classType = nullptr;
            if (parent->IsClassTypeSymbol())
            {
                classType = static_cast<ClassTypeSymbol*>(parent);
            }
            else
            {
                throw std::runtime_error("not class type");
            }
            if (IsConstructor())
            {
                syntax.append(classType->Name());
            }
            else if (IsDestructor())
            {
                syntax.append("~").append(classType->Name());
            }
            else
            {
                syntax.append(groupName);
            }
        }
        else
        {
            syntax.append(groupName);
        }
        syntax.append("(");
        int n = int(Parameters().size());
        for (int i = startParamIndex; i < n; ++i)
        {
            if (i > startParamIndex)
            {
                syntax.append(", ");
            }
            ParameterSymbol* param = Parameters()[i];
            syntax.append(param->GetType()->FullName()).append(" ").append(param->Name());
        }
        syntax.append(")");
        if (IsConst())
        {
            syntax.append(" const");
        }
    }
    syntax.append(";");
    return syntax;
}

std::string FunctionSymbol::ParsingName() const
{
    std::string parsingName;
    if (IsIntrinsic())
    {
        parsingName.append(groupName);
        if (typeParameters.size() > 0)
        {
            parsingName.append(1, '<');
            bool first = true;
            for (TypeParameterSymbol* typeParam : typeParameters)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    parsingName.append(", ");
                }
                parsingName.append(typeParam->Name());
            }
            parsingName.append(1, '>');
        }
        parsingName.append("(");
        int n = int(Parameters().size());
        for (int i = 0; i < n; ++i)
        {
            if (i > 0)
            {
                parsingName.append(", ");
            }
            ParameterSymbol* param = Parameters()[i];
            parsingName.append(param->GetType()->FullName());
        }
        parsingName.append(")");
    }
    else
    {
        int startParamIndex = 0;
        if (IsMemberFunctionSymbol())
        {
            if (!IsStatic())
            {
                startParamIndex = 1;
            }
            Symbol* parent = Parent();
            TypeSymbol* parentType = nullptr;
            if (parent->IsClassTypeSymbol())
            {
                parentType = static_cast<TypeSymbol*>(parent);
            }
            else if (parent->IsInterfaceTypeSymbol())
            {
                parentType = static_cast<TypeSymbol*>(parent);
            }
            else
            {
                throw std::runtime_error("not class or interface type");
            }
            if (IsConstructor())
            {
                parsingName.append(parentType->Name());
            }
            else if (IsDestructor())
            {
                parsingName.append("~").append(parentType->Name());
            }
            else
            {
                parsingName.append(groupName);
            }
        }
        else
        {
            parsingName.append(groupName);
        }
        parsingName.append("(");
        int n = int(Parameters().size());
        for (int i = startParamIndex; i < n; ++i)
        {
            if (i > startParamIndex)
            {
                parsingName.append(", ");
            }
            ParameterSymbol* param = Parameters()[i];
            parsingName.append(param->GetType()->FullName());
        }
        parsingName.append(")");
        if (IsConst())
        {
            parsingName.append(" const");
        }
    }
    return parsingName;
}

std::string FunctionSymbol::CCName() const
{
    if (IsFunctionTemplate())
    {
        return groupName;
    }
    else
    {
        std::string ccName;
        int startParamIndex = 0;
        if (IsMemberFunctionSymbol())
        {
            if (!IsStatic())
            {
                startParamIndex = 1;
            }
            Symbol* parent = Parent();
            TypeSymbol* parentType = nullptr;
            if (parent->IsClassTypeSymbol())
            {
                parentType = static_cast<TypeSymbol*>(parent);
            }
            else if (parent->IsInterfaceTypeSymbol())
            {
                parentType = static_cast<TypeSymbol*>(parent);
            }
            else
            {
                throw std::runtime_error("not class or interface type");
            }
            if (IsConstructor())
            {
                ccName.append(parentType->Name());
            }
            else if (IsDestructor())
            {
                ccName.append("~").append(parentType->Name());
            }
            else
            {
                ccName.append(groupName);
            }
        }
        else
        {
            ccName.append(groupName);
        }
        return ccName;
    }
}

std::string FunctionSymbol::FullCCName(SymbolTable& symbolTable) 
{
    std::string fullCCName;
    std::string parentFullName;
    Symbol* p = Parent();   // Parent(): derived type returns base type's parent if it has not parent of its own
    if (p)
    {
        parentFullName = p->FullName();
    }
    if (!parentFullName.empty())
    {
        parentFullName.append(1, '.');
    }
    if (IsFunctionTemplate())
    {
        if (IsIntrinsic())
        {
            fullCCName.append(returnType->FullName()).append(1, ' ').append(parentFullName).append(groupName);
            int m = int(typeParameters.size());
            if (m > 0)
            {
                fullCCName.append(1, '<');
                for (int i = 0; i < m; ++i)
                {
                    if (i > 0)
                    {
                        fullCCName.append(", ");
                    }
                    fullCCName.append(typeParameters[i]->Name());
                }
                fullCCName.append(1, '>');
            }
            fullCCName.append(1, '(');
            int n = int(parameters.size());
            for (int i = 0; i < n; ++i)
            {
                if (i > 0)
                {
                    fullCCName.append(", ");
                }
                ParameterSymbol* parameter = parameters[i];
                fullCCName.append(parameter->GetType()->FullName()).append(1, ' ').append(parameter->Name());
            }
            fullCCName.append(1, ')');
        }
        else
        {
            Cm::Ast::Node* node = symbolTable.GetNode(this, false);
            if (!node)
            {
                ReadFunctionNode(symbolTable, GetSpan().FileIndex());
                node = symbolTable.GetNode(this);
            }
            if (node->IsFunctionNode())
            {
                Cm::Ast::FunctionNode* functionNode = static_cast<Cm::Ast::FunctionNode*>(node);
                fullCCName.append(functionNode->ReturnTypeExpr()->ToString()).append(1, ' ');
                fullCCName.append(parentFullName).append(groupName);
                int m = functionNode->TemplateParameters().Count();
                if (m > 0)
                {
                    fullCCName.append(1, '<');
                    for (int i = 0; i < m; ++i)
                    {
                        if (i > 0)
                        {
                            fullCCName.append(", ");
                        }
                        fullCCName.append(functionNode->TemplateParameters()[i]->ToString());
                    }
                    fullCCName.append(1, '>');
                }
                fullCCName.append(1, '(');
                int n = functionNode->Parameters().Count();
                for (int i = 0; i < n; ++i)
                {
                    if (i > 0)
                    {
                        fullCCName.append(", ");
                    }
                    Cm::Ast::ParameterNode* parameter = functionNode->Parameters()[i];
                    fullCCName.append(parameter->ToString());
                }
                fullCCName.append(1, ')');
                if (functionNode->Constraint())
                {
                    fullCCName.append(1, ' ').append(functionNode->Constraint()->ToString());
                }
            }
            else
            {
                throw std::runtime_error("function node expected");
            }
        }
    }
    else
    {
        std::string returnTypeName;
        if (returnType)
        {
            returnTypeName.append(returnType->FullName()).append(1, ' ');
        }
        std::string commonName = groupName;
        if (IsConstructor())
        {
            commonName = Class()->Name();
        }
        fullCCName.append(returnTypeName).append(parentFullName).append(commonName).append(1, '(');
        int startParamIndex = 0;
        if (IsMemberFunctionSymbol())
        {
            if (!IsStatic())
            {
                startParamIndex = 1;
            }
        }
        bool first = true;
        int n = int(parameters.size());
        for (int i = startParamIndex; i < n; ++i)
        { 
            if (first)
            {
                first = false;
            }
            else
            {
                fullCCName.append(", ");
            }
            ParameterSymbol* parameter = parameters[i];
            fullCCName.append(parameter->GetType()->FullName()).append(1, ' ').append(parameter->Name());
        }
        fullCCName.append(1, ')');
        if (IsConst())
        {
            fullCCName.append(" const");
        }
    }
    return fullCCName;
}

bool FunctionSymbol::IsCCSymbol() const 
{
    if (IsDestructor() || IsStaticConstructor())
    {
        return false;
    }
    return true;
}

TypeSymbol* FunctionSymbol::GetTargetType() const
{
    if (IsConvertingConstructor())
    {
        return parameters[0]->GetType()->GetBaseType();
    }
    else if (IsConversionFunction())
    {
        return returnType;
    }
    else
    {
        throw std::runtime_error("not converting constructor or conversion function");
    }
}

TypeSymbol* FunctionSymbol::GetSourceType() const
{
    if (IsConvertingConstructor())
    {
        return parameters[1]->GetType()->GetBaseType();
    }
    else if (IsConversionFunction())
    {
        return parameters[0]->GetType()->GetBaseType();
    }
    else
    {
        throw std::runtime_error("not converting constructor or conversion function");
    }
}

void FunctionSymbol::CollectExportedDerivedTypes(std::unordered_set<Symbol*>& collected, std::unordered_set<TypeSymbol*>& exportedDerivedTypes)
{
    if (returnType)
    {
        if (returnType->IsDerivedTypeSymbol())
        {
            if (collected.find(returnType) == collected.end())
            {
                collected.insert(returnType);
                returnType->CollectExportedDerivedTypes(collected, exportedDerivedTypes);
            }
        }
    }
    for (ParameterSymbol* parameter : parameters)
    {
        parameter->CollectExportedDerivedTypes(collected, exportedDerivedTypes);
    }
}

void FunctionSymbol::CollectExportedTemplateTypes(std::unordered_set<Symbol*>& collected, std::unordered_map<TypeId, std::unordered_set<TemplateTypeSymbol*>, TypeIdHash>& exportedTemplateTypes)
{
    if (returnType)
    {
        if (collected.find(returnType) == collected.end())
        {
            collected.insert(returnType);
            returnType->CollectExportedTemplateTypes(collected, exportedTemplateTypes);
        }
    }
    for (ParameterSymbol* parameter : parameters)
    {
        parameter->CollectExportedTemplateTypes(collected, exportedTemplateTypes);
    }
}

void FunctionSymbol::Dump(CodeFormatter& formatter)
{
    if (!IsProject()) return;
    if (IsBasicTypeOp()) return;
    std::string s = SymbolFlagStr(Flags(), DeclaredAccess(), true);
    std::string fs = FunctionSymbolFlagString(flags);
    if (!fs.empty())
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append(fs);
    }
    if (returnType)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append(returnType->FullName());
    }
    if (!s.empty())
    {
        s.append(1, ' ');
    }
    s.append(groupName);
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
        s.append(t);
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
    s.append(p);
    formatter.WriteLine(s);
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

void FunctionSymbol::SetGlobalNs(Cm::Ast::NamespaceNode* globalNs_)
{
    globalNs.reset(globalNs_);
}

void FunctionSymbol::AddToOverrideSet(FunctionSymbol* overrideFun)
{
    overrideSet.insert(overrideFun);
    if (overriddenFunction)
    {
        overriddenFunction->AddToOverrideSet(overrideFun);
    }
}

void FunctionSymbol::SetConstraintDocId(const std::string& constraintDocId_)
{
    constraintDocId = constraintDocId_;
}

void FunctionSymbol::ReplaceReplicaTypes()
{
    if (IsFunctionTemplate()) return;
    ContainerSymbol::ReplaceReplicaTypes();
    if (returnType && returnType->IsReplica() && returnType->IsTemplateTypeSymbol())
    {
        TemplateTypeSymbol* replica = static_cast<TemplateTypeSymbol*>(returnType);
        returnType = replica->GetPrimaryTemplateTypeSymbol();
    }
    for (TypeSymbol*& typeSymbol : typeArguments)
    {
        if (typeSymbol->IsReplica() && typeSymbol->IsTemplateTypeSymbol())
        {
            TemplateTypeSymbol* replica = static_cast<TemplateTypeSymbol*>(typeSymbol);
            typeSymbol = replica->GetPrimaryTemplateTypeSymbol();
        }
    }
}

void FunctionSymbol::DoSerialize()
{
    ContainerSymbol::DoSerialize();
    if (returnType)
    {
        returnType->DoSerialize();
    }
    for (ParameterSymbol* parameter : parameters)
    {
        parameter->DoSerialize();
    }
}

} } // namespace Cm::Sym
