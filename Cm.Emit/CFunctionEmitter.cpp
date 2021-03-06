/*========================================================================
    Copyright (c) 2012-2016 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Emit/CFunctionEmitter.hpp>
#include <Cm.Emit/SourceFileCache.hpp>
#include <Cm.BoundTree/BoundFunction.hpp>
#include <Cm.Parser/FileRegistry.hpp>
#include <Cm.Sym/GlobalFlags.hpp>
#include <Cm.Sym/TemplateTypeSymbol.hpp>
#include <Cm.Sym/BasicTypeSymbol.hpp>
#include <Cm.IrIntf/Rep.hpp>

namespace Cm { namespace Emit {

CFunctionEmitter::CFunctionEmitter(Cm::Util::CodeFormatter& codeFormatter_, Cm::Sym::TypeRepository& typeRepository_, Cm::Core::IrFunctionRepository& irFunctionRepository_,
    Cm::Core::IrClassTypeRepository& irClassTypeRepository_, Cm::Core::StringRepository& stringRepository_, Cm::BoundTree::BoundClass* currentClass_,
    std::unordered_set<std::string>& internalFunctionNames_, std::unordered_set<Ir::Intf::Function*>& externalFunctions_,
    Cm::Core::StaticMemberVariableRepository& staticMemberVariableRepository_, Cm::Core::ExternalConstantRepository& externalConstantRepository_,
    Cm::Ast::CompileUnitNode* currentCompileUnit_, Cm::Sym::FunctionSymbol* enterFrameFun_, Cm::Sym::FunctionSymbol* leaveFrameFun_, Cm::Sym::FunctionSymbol* enterTracedCalllFun_,
    Cm::Sym::FunctionSymbol* leaveTracedCallFun_, Cm::Sym::FunctionSymbol* interfaceLookupFailed_, const char* start_, const char* end_, bool generateDebugInfo_, bool profile_) :
    FunctionEmitter(codeFormatter_, typeRepository_, irFunctionRepository_, irClassTypeRepository_, stringRepository_, currentClass_,
    internalFunctionNames_, externalFunctions_, staticMemberVariableRepository_, externalConstantRepository_, currentCompileUnit_, enterFrameFun_, leaveFrameFun_, enterTracedCalllFun_,
    leaveTracedCallFun_, interfaceLookupFailed_, generateDebugInfo_, profile_), functionMap(nullptr), start(start_), end(end_), generateDebugInfo(generateDebugInfo_)
{
}

void CFunctionEmitter::BeginVisit(Cm::BoundTree::BoundFunction& boundFunction)
{
    FunctionEmitter::BeginVisit(boundFunction);
    if (GenerateDebugInfo())
    {
        Cm::Sym::FunctionSymbol* currentFunctionSymbol = boundFunction.GetFunctionSymbol();
        functionDebugInfo.reset(new Cm::Core::CFunctionDebugInfo(IrFunctionRepository().CreateIrFunction(currentFunctionSymbol)->Name()));
        functionDebugInfo->SetFunctionDisplayName(currentFunctionSymbol->FullName());
        functionDebugInfo->SetCFilePath(cFilePath);
        if (currentFunctionSymbol->GroupName() == "main")
        {
            functionDebugInfo->SetMain();
        }
        if (!currentFunctionSymbol->IsReplicated())
        {
            functionDebugInfo->SetUnique();
        }
        if (currentFunctionSymbol->Parent() && currentFunctionSymbol->Parent()->IsTemplateTypeSymbol())
        {
            Cm::Sym::TemplateTypeSymbol* templateTypeSymbol = static_cast<Cm::Sym::TemplateTypeSymbol*>(currentFunctionSymbol->Parent());
            Cm::Sym::TypeSymbol* subjectType = templateTypeSymbol->GetSubjectType();
            if (!subjectType->IsClassTypeSymbol())
            {
                throw std::runtime_error("subject type not class type");
            }
            Cm::Sym::ClassTypeSymbol* subjectClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(subjectType);
            const std::string& sourceFilePath = subjectClassType->SourceFilePath();
            if (sourceFilePath.empty())
            {
                throw std::runtime_error("template type symbol source file path not set");
            }
            functionDebugInfo->SetSourceFilePath(sourceFilePath);
            SourceFile& sourceFile = SourceFileCache::Instance().GetSourceFile(sourceFilePath);
            start = sourceFile.Begin();
            end = sourceFile.End();
        }
        else if (currentFunctionSymbol->CompileUnit())
        {
            functionDebugInfo->SetSourceFilePath(currentFunctionSymbol->CompileUnit()->FilePath());
        }
        else
        {
            int32_t fileIndex = currentFunctionSymbol->GetSpan().FileIndex();
            const std::string& sourceFilePath = Cm::Parser::FileRegistry::Instance()->GetParsedFileName(fileIndex);
            functionDebugInfo->SetSourceFilePath(sourceFilePath);
            SourceFile& sourceFile = SourceFileCache::Instance().GetSourceFile(sourceFilePath);
            start = sourceFile.Begin();
            end = sourceFile.End();
        }
    }
    std::unordered_set<Ir::Intf::Type*> functionPtrTypes;
    std::unordered_map<Ir::Intf::Type*, Ir::Intf::Type*> tdfMap;
    GetLocalVariableIrObjectRepository().GetFunctionPtrTypes(functionPtrTypes);
    if (!functionPtrTypes.empty())
    {
        for (Ir::Intf::Type* functionPtrType : functionPtrTypes)
        {
            std::unique_ptr<C::Typedef> tdf(new C::Typedef(Ir::Intf::GetCurrentTempTypedefProvider()->GetNextTempTypedefName(), functionPtrType->Clone()));
            tdfMap[functionPtrType] = tdf.get();
            tdfs.push_back(std::move(tdf));
        }
        GetLocalVariableIrObjectRepository().ReplaceFunctionPtrTypes(tdfMap);
    }
}

Ir::Intf::Type* CFunctionEmitter::ReplaceFunctionPtrType(Ir::Intf::Type* localVariableIrType)
{
    std::unordered_set<Ir::Intf::Type*> functionPtrTypes;
    std::unordered_map<Ir::Intf::Type*, Ir::Intf::Type*> tdfMap;
    localVariableIrType->GetFunctionPtrTypes(functionPtrTypes);
    if (!functionPtrTypes.empty())
    {
        for (Ir::Intf::Type* functionPtrType : functionPtrTypes)
        {
            std::unique_ptr<C::Typedef> tdf(new C::Typedef(Ir::Intf::GetCurrentTempTypedefProvider()->GetNextTempTypedefName(), functionPtrType->Clone()));
            tdfMap[functionPtrType] = tdf.get();
            tdfs.push_back(std::move(tdf));
        }
        localVariableIrType->ReplaceFunctionPtrTypes(tdfMap);
    }
    return localVariableIrType;
}

void CFunctionEmitter::EndVisit(Cm::BoundTree::BoundFunction& boundFunction)
{
    FunctionEmitter::EndVisit(boundFunction);
    C::Function* function = static_cast<C::Function*>(Emitter()->GetIrFunction());
    std::vector<std::unique_ptr<C::Typedef>> funTdfs = std::move(function->Tdfs());
    for (std::unique_ptr<C::Typedef>& tdf : funTdfs)
    {
        tdfs.push_back(std::move(tdf));
    }
}

void CFunctionEmitter::EmitDummyVar(Cm::Core::Emitter* emitter)
{
    Ir::Intf::Object* dummyVariable = Cm::IrIntf::CreateStackVar("_X_dummy", Cm::IrIntf::Pointer(Ir::Intf::GetFactory()->GetI1(), 1));
    emitter->Own(dummyVariable);
    emitter->Emit(Cm::IrIntf::Alloca(Ir::Intf::GetFactory()->GetI1(), dummyVariable));
}

void CFunctionEmitter::SetStringLiteralResult(Cm::Core::Emitter* emitter, Ir::Intf::Object* resultObject, Ir::Intf::Object* stringConstant, Ir::Intf::Object* stringObject)
{
    Cm::IrIntf::Assign(*emitter, stringConstant->GetType(), stringObject, resultObject);
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundDynamicTypeNameExpression& boundDynamicTypeNameExpression)
{
    Cm::Core::Emitter* emitter = Emitter();
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(emitter, GenFlags()));
    boundDynamicTypeNameExpression.Subject()->Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> subjectResult = ResultStack().Pop();
    Ir::Intf::LabelObject* resultLabel = subjectResult->GetLabel();
    if (resultLabel)
    {
        result->SetLabel(resultLabel);
    }
    Cm::Sym::ClassTypeSymbol* classType = boundDynamicTypeNameExpression.ClassType();
    Ir::Intf::Type* classTypePtrType = Cm::IrIntf::Pointer(classType->GetIrType(), 1);
    emitter->Own(classTypePtrType);
    Ir::Intf::Object* objectPtr = subjectResult->MainObject();
    Ir::Intf::Type* voidPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1));
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2));
    emitter->Own(voidPtrPtr);
    Ir::Intf::Object* vtblPtrContainerPtr = objectPtr;
    int vptrIndex = classType->VPtrIndex();
    if (vptrIndex == -1)
    {
        Cm::Sym::ClassTypeSymbol* vptrContainerClass = classType->VPtrContainerClass();
        vptrIndex = vptrContainerClass->VPtrIndex();
        Ir::Intf::Type* containerPtrType = Cm::IrIntf::Pointer(vptrContainerClass->GetIrType(), 1);
        emitter->Own(containerPtrType);
        Ir::Intf::RegVar* containerPtr = Cm::IrIntf::CreateTemporaryRegVar(containerPtrType);
        emitter->Own(containerPtr);
        emitter->Emit(Cm::IrIntf::Bitcast(classTypePtrType, containerPtr, objectPtr, containerPtrType));
        vtblPtrContainerPtr = containerPtr;
    }
    Ir::Intf::MemberVar* vptr = Cm::IrIntf::CreateMemberVar(Cm::IrIntf::GetVPtrVarName(), vtblPtrContainerPtr, vptrIndex, voidPtrPtr);
    emitter->Own(vptr);
    Ir::Intf::RegVar* loadedVptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(loadedVptr);
    Cm::IrIntf::Assign(*emitter, voidPtrPtr, vptr, loadedVptr);
    Ir::Intf::RegVar* voidrttiPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(voidrttiPtr);
    Cm::IrIntf::Assign(*emitter, voidPtr, loadedVptr, voidrttiPtr);
    Ir::Intf::Type* rttiPtrIrType(Cm::IrIntf::Pointer(Cm::IrIntf::CreateTypeName("rtti", false), 1));
    emitter->Own(rttiPtrIrType);
    Ir::Intf::RegVar* rttiPtr = Cm::IrIntf::CreateTemporaryRegVar(rttiPtrIrType);
    emitter->Own(rttiPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, rttiPtr, voidrttiPtr, rttiPtrIrType));
    Ir::Intf::Type* charPtrPtrType = Cm::IrIntf::Pointer(Cm::IrIntf::Char(), 2);
    emitter->Own(charPtrPtrType);
    Ir::Intf::RegVar* typenameCharPtrPtr = Cm::IrIntf::CreateTemporaryRegVar(charPtrPtrType);
    emitter->Own(typenameCharPtrPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtrPtr, typenameCharPtrPtr, rttiPtr, charPtrPtrType));
    Ir::Intf::Type* charPtrType = Cm::IrIntf::Pointer(Cm::IrIntf::Char(), 1);
    emitter->Own(charPtrType);
    Ir::Intf::RegVar* loadedTypenameCharPtr = Cm::IrIntf::CreateTemporaryRegVar(charPtrType);
    emitter->Own(loadedTypenameCharPtr);
    Cm::IrIntf::Assign(*emitter, charPtrType, typenameCharPtrPtr, loadedTypenameCharPtr);
    result->SetMainObject(loadedTypenameCharPtr);
    result->Merge(subjectResult);
    ResultStack().Push(result);
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundIsExpression& boundIsExpression)
{
    Cm::Core::Emitter* emitter = Emitter();
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(emitter, GenFlags()));
    boundIsExpression.Expr()->Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> exprResult = ResultStack().Pop();
    Ir::Intf::LabelObject* resultLabel = exprResult->GetLabel();
    if (resultLabel)
    {
        result->SetLabel(resultLabel);
    }
    Cm::Sym::ClassTypeSymbol* leftClassType = boundIsExpression.LeftClassType();
    Ir::Intf::Type* classTypePtrType = Cm::IrIntf::Pointer(leftClassType->GetIrType(), 1);
    emitter->Own(classTypePtrType);
    Ir::Intf::Object* objectPtr = exprResult->MainObject();
    Ir::Intf::Type* voidPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1));
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2));
    emitter->Own(voidPtrPtr);
    Ir::Intf::Object* vtblPtrContainerPtr = objectPtr;
    int vptrIndex = leftClassType->VPtrIndex();
    if (vptrIndex == -1)
    {
        Cm::Sym::ClassTypeSymbol* vptrContainerClass = leftClassType->VPtrContainerClass();
        vptrIndex = vptrContainerClass->VPtrIndex();
        Ir::Intf::Type* containerPtrType = Cm::IrIntf::Pointer(vptrContainerClass->GetIrType(), 1);
        emitter->Own(containerPtrType);
        Ir::Intf::RegVar* containerPtr = Cm::IrIntf::CreateTemporaryRegVar(containerPtrType);
        emitter->Own(containerPtr);
        emitter->Emit(Cm::IrIntf::Bitcast(classTypePtrType, containerPtr, objectPtr, containerPtrType));
        vtblPtrContainerPtr = containerPtr;
    }
    Ir::Intf::MemberVar* vptr = Cm::IrIntf::CreateMemberVar(Cm::IrIntf::GetVPtrVarName(), vtblPtrContainerPtr, vptrIndex, voidPtrPtr);
    emitter->Own(vptr);
    Ir::Intf::RegVar* loadedVptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(loadedVptr);
    Cm::IrIntf::Assign(*emitter, voidPtrPtr, vptr, loadedVptr);
    Ir::Intf::RegVar* voidrttiPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(voidrttiPtr);
    Cm::IrIntf::Assign(*emitter, voidPtr, loadedVptr, voidrttiPtr);
    Ir::Intf::Type* rttiPtrIrType(Cm::IrIntf::Pointer(Cm::IrIntf::CreateTypeName("rtti", false), 1));
    emitter->Own(rttiPtrIrType);
    Ir::Intf::RegVar* rttiPtr = Cm::IrIntf::CreateTemporaryRegVar(rttiPtrIrType);
    emitter->Own(rttiPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, rttiPtr, voidrttiPtr, rttiPtrIrType));
    Ir::Intf::Type* cidPtrType = Cm::IrIntf::Pointer(Ir::Intf::GetFactory()->GetUI64(), 1);
    emitter->Own(cidPtrType);
    Ir::Intf::MemberVar* cidMemberVar = Cm::IrIntf::CreateMemberVar("class_id", rttiPtr, 1, cidPtrType);
    emitter->Own(cidMemberVar);
    Ir::Intf::RegVar* leftCid = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetUI64());
    emitter->Own(leftCid);
    Cm::IrIntf::Assign(*emitter, Ir::Intf::GetFactory()->GetUI64(), cidMemberVar, leftCid);
    Cm::Sym::ClassTypeSymbol* rightClassType = boundIsExpression.RightClassType();
    Cm::BoundTree::BoundLiteral rightCidLiteral(boundIsExpression.SyntaxNode());
    rightCidLiteral.SetValue(new Cm::Sym::ULongValue(rightClassType->Cid()));
    Cm::Sym::TypeSymbol* ulongType = GetSymbolTable()->GetTypeRepository().GetType(Cm::Sym::GetBasicTypeId(Cm::Sym::ShortBasicTypeId::ulongId));
    rightCidLiteral.SetType(ulongType);
    rightCidLiteral.Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> rightCidResult = ResultStack().Pop();
    Ir::Intf::Object* rightCid = rightCidResult->MainObject();
    Ir::Intf::RegVar* resultRegVar = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetI1());
    emitter->Own(resultRegVar);
    if (Cm::Sym::GetGlobalFlag(Cm::Sym::GlobalFlags::fullConfig))
    {
        Ir::Intf::RegVar* remainderResult = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetUI64());
        emitter->Emit(Cm::IrIntf::URem(Ir::Intf::GetFactory()->GetUI64(), remainderResult, leftCid, rightCid));
        Ir::Intf::Object* ui64Zero = Ir::Intf::GetFactory()->GetUI64()->CreateDefaultValue();
        emitter->Own(ui64Zero);
        emitter->Emit(Cm::IrIntf::ICmp(Ir::Intf::GetFactory()->GetUI64(), resultRegVar, Ir::Intf::IConditionCode::eq, remainderResult, ui64Zero));
    }
    else
    {
        Cm::Sym::FunctionSymbol* fun = GetSymbolTable()->GetOverload("is_class_same_or_derived_from");
        std::shared_ptr<Cm::Core::GenResult> funCallResult(new Cm::Core::GenResult(emitter, GenFlags()));
        funCallResult->SetMainObject(resultRegVar);
        funCallResult->AddObject(leftCid);
        funCallResult->AddObject(rightCid);
        GenerateCall(fun, nullptr, *funCallResult);
    }
    result->SetMainObject(resultRegVar);
    result->Merge(exprResult);
    result->Merge(rightCidResult);
    if (boundIsExpression.GetFlag(Cm::BoundTree::BoundNodeFlags::genJumpingBoolCode))
    {
        GenJumpingBoolCode(*result);
    }
    ResultStack().Push(result);
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundAsExpression& boundAsExpression)
{
    Cm::Core::Emitter* emitter = Emitter();
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(emitter, GenFlags()));
    boundAsExpression.Expr()->Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> exprResult = ResultStack().Pop();
    Ir::Intf::LabelObject* resultLabel = exprResult->GetLabel();
    if (resultLabel)
    {
        result->SetLabel(resultLabel);
    }
    Cm::Sym::ClassTypeSymbol* leftClassType = boundAsExpression.LeftClassType();
    Ir::Intf::Type* classTypePtrType = Cm::IrIntf::Pointer(leftClassType->GetIrType(), 1);
    emitter->Own(classTypePtrType);
    Ir::Intf::Object* objectPtr = exprResult->MainObject();
    Ir::Intf::Type* voidPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1));
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr(Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2));
    emitter->Own(voidPtrPtr);
    Ir::Intf::Object* vtblPtrContainerPtr = objectPtr;
    int vptrIndex = leftClassType->VPtrIndex();
    if (vptrIndex == -1)
    {
        Cm::Sym::ClassTypeSymbol* vptrContainerClass = leftClassType->VPtrContainerClass();
        vptrIndex = vptrContainerClass->VPtrIndex();
        Ir::Intf::Type* containerPtrType = Cm::IrIntf::Pointer(vptrContainerClass->GetIrType(), 1);
        emitter->Own(containerPtrType);
        Ir::Intf::RegVar* containerPtr = Cm::IrIntf::CreateTemporaryRegVar(containerPtrType);
        emitter->Own(containerPtr);
        emitter->Emit(Cm::IrIntf::Bitcast(classTypePtrType, containerPtr, objectPtr, containerPtrType));
        vtblPtrContainerPtr = containerPtr;
    }
    Ir::Intf::MemberVar* vptr = Cm::IrIntf::CreateMemberVar(Cm::IrIntf::GetVPtrVarName(), vtblPtrContainerPtr, vptrIndex, voidPtrPtr);
    emitter->Own(vptr);
    Ir::Intf::RegVar* loadedVptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(loadedVptr);
    Cm::IrIntf::Assign(*emitter, voidPtrPtr, vptr, loadedVptr);
    Ir::Intf::RegVar* voidrttiPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(voidrttiPtr);
    Cm::IrIntf::Assign(*emitter, voidPtr, loadedVptr, voidrttiPtr);
    Ir::Intf::Type* rttiPtrIrType(Cm::IrIntf::Pointer(Cm::IrIntf::CreateTypeName("rtti", false), 1));
    emitter->Own(rttiPtrIrType);
    Ir::Intf::RegVar* rttiPtr = Cm::IrIntf::CreateTemporaryRegVar(rttiPtrIrType);
    emitter->Own(rttiPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, rttiPtr, voidrttiPtr, rttiPtrIrType));
    Ir::Intf::Type* cidPtrType = Cm::IrIntf::Pointer(Ir::Intf::GetFactory()->GetUI64(), 1);
    emitter->Own(cidPtrType);
    Ir::Intf::MemberVar* cidMemberVar = Cm::IrIntf::CreateMemberVar("class_id", rttiPtr, 1, cidPtrType);
    emitter->Own(cidMemberVar);
    Ir::Intf::RegVar* leftCid = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetUI64());
    emitter->Own(leftCid);
    Cm::IrIntf::Assign(*emitter, Ir::Intf::GetFactory()->GetUI64(), cidMemberVar, leftCid);
    Cm::Sym::ClassTypeSymbol* rightClassType = boundAsExpression.RightClassType();
    Cm::BoundTree::BoundLiteral rightCidLiteral(boundAsExpression.SyntaxNode());
    rightCidLiteral.SetValue(new Cm::Sym::ULongValue(rightClassType->Cid()));
    Cm::Sym::TypeSymbol* ulongType = GetSymbolTable()->GetTypeRepository().GetType(Cm::Sym::GetBasicTypeId(Cm::Sym::ShortBasicTypeId::ulongId));
    rightCidLiteral.SetType(ulongType);
    rightCidLiteral.Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> rightCidResult = ResultStack().Pop();
    Ir::Intf::Object* rightCid = rightCidResult->MainObject();
    Ir::Intf::RegVar* resultRegVar = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetI1());
    emitter->Own(resultRegVar);
    if (Cm::Sym::GetGlobalFlag(Cm::Sym::GlobalFlags::fullConfig))
    {
        Ir::Intf::RegVar* remainderResult = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetUI64());
        emitter->Emit(Cm::IrIntf::URem(Ir::Intf::GetFactory()->GetUI64(), remainderResult, leftCid, rightCid));
        Ir::Intf::Object* ui64Zero = Ir::Intf::GetFactory()->GetUI64()->CreateDefaultValue();
        emitter->Own(ui64Zero);
        emitter->Emit(Cm::IrIntf::ICmp(Ir::Intf::GetFactory()->GetUI64(), resultRegVar, Ir::Intf::IConditionCode::eq, remainderResult, ui64Zero));
    }
    else
    {
        Cm::Sym::FunctionSymbol* fun = GetSymbolTable()->GetOverload("is_class_same_or_derived_from");
        std::shared_ptr<Cm::Core::GenResult> funCallResult(new Cm::Core::GenResult(emitter, GenFlags()));
        funCallResult->SetMainObject(resultRegVar);
        funCallResult->AddObject(leftCid);
        funCallResult->AddObject(rightCid);
        GenerateCall(fun, nullptr, *funCallResult);
    }
    Ir::Intf::LabelObject* trueLabel = Cm::IrIntf::CreateNextLocalLabel();
    emitter->Own(trueLabel);
    Ir::Intf::LabelObject* falseLabel = Cm::IrIntf::CreateNextLocalLabel();
    emitter->Own(falseLabel);
    Ir::Intf::LabelObject* exitLabel = Cm::IrIntf::CreateNextLocalLabel();
    emitter->Own(exitLabel);
    emitter->Emit(Cm::IrIntf::Br(resultRegVar, trueLabel, falseLabel));
    Cm::BoundTree::BoundExpression* temporary = boundAsExpression.BoundTemporary();
    temporary->Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> temporaryResult = ResultStack().Pop();
    emitter->AddNextInstructionLabel(trueLabel);
    Ir::Intf::Object* exprAsRightOperand = Cm::IrIntf::CreateTemporaryRegVar(boundAsExpression.GetType()->GetIrType());
    emitter->Emit(Cm::IrIntf::Bitcast(exprResult->MainObject()->GetType(), exprAsRightOperand, exprResult->MainObject(), boundAsExpression.GetType()->GetIrType()));
    emitter->Emit(Cm::IrIntf::Store(boundAsExpression.GetType()->GetIrType(), exprAsRightOperand, temporaryResult->MainObject()));
    emitter->Emit(Cm::IrIntf::Br(exitLabel));
    Ir::Intf::Object* null = boundAsExpression.GetType()->GetIrType()->CreateDefaultValue();
    emitter->AddNextInstructionLabel(falseLabel);
    emitter->Emit(Cm::IrIntf::Store(boundAsExpression.GetType()->GetIrType(), null, temporaryResult->MainObject()));
    emitter->AddNextInstructionLabel(exitLabel);
    result->SetMainObject(GetSymbolTable()->GetTypeRepository().GetType(Cm::Sym::GetBasicTypeId(Cm::Sym::ShortBasicTypeId::voidId)), GetSymbolTable()->GetTypeRepository());
    DoNothing(*result);
    result->SetMainObject(temporaryResult->MainObject());
    result->Merge(exprResult);
    result->Merge(rightCidResult);
    result->Merge(temporaryResult);
    ResultStack().Push(result);
}

void CFunctionEmitter::DoNothing(Cm::Core::GenResult& genResult)
{
    Emitter()->Emit(Cm::IrIntf::DoNothing());
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundInitVPtrStatement& boundInitVPtrStatement)
{
    Cm::Core::Emitter* emitter = Emitter();
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(emitter, GenFlags()));
    Ir::Intf::Type* voidPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1);
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2);
    emitter->Own(voidPtrPtr);
    Cm::Sym::ClassTypeSymbol* classType = boundInitVPtrStatement.ClassType();
    Cm::BoundTree::BoundParameter boundThisParam(nullptr, ThisParam());
    boundThisParam.Accept(*this);
    std::shared_ptr<Cm::Core::GenResult> thisResult = ResultStack().Pop();
    int16_t vptrIndex = classType->VPtrIndex();
    Ir::Intf::Object* vptrContainerPtr = thisResult->MainObject();
    if (vptrIndex == -1)
    {
        Cm::Sym::ClassTypeSymbol* vptrContainingType = classType->VPtrContainerClass();
        vptrIndex = vptrContainingType->VPtrIndex();
        Ir::Intf::Type* vptrContainingTypeIrType = vptrContainingType->GetIrType();
        Ir::Intf::Type* vptrContainingTypePtrType = Cm::IrIntf::Pointer(vptrContainingTypeIrType, 1);
        emitter->Own(vptrContainingTypePtrType);
        Ir::Intf::RegVar* containerPtr = Cm::IrIntf::CreateTemporaryRegVar(vptrContainingTypePtrType);
        emitter->Own(containerPtr);
        Ir::Intf::Type* classTypeIrType = classType->GetIrType();
        Ir::Intf::Type* classTypePtrType = Cm::IrIntf::Pointer(classTypeIrType, 1);
        emitter->Own(classTypePtrType);
        emitter->Emit(Cm::IrIntf::Bitcast(classTypePtrType, containerPtr, thisResult->MainObject(), vptrContainingTypePtrType));
        vptrContainerPtr = containerPtr;
    }
    Ir::Intf::MemberVar* vptr = Cm::IrIntf::CreateMemberVar(Cm::IrIntf::GetVPtrVarName(), vptrContainerPtr, vptrIndex, voidPtrPtr);
    emitter->Own(vptr);
    Ir::Intf::RegVar* vtblAsVoidPtrPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(vtblAsVoidPtrPtr);
    Ir::Intf::Type* vtblAddrType = Cm::IrIntf::Pointer(Cm::IrIntf::Array(voidPtr->Clone(), int(classType->Vtbl().size())), 1);
    emitter->Own(vtblAddrType);
    std::string vtblName = Cm::IrIntf::MakeAssemblyName(classType->FullName() + Cm::IrIntf::GetPrivateSeparator() + "vtbl");
    Ir::Intf::Object* vtblObject = Cm::IrIntf::CreateGlobal(vtblName, vtblAddrType);
    emitter->Own(vtblObject);
    emitter->Emit(Cm::IrIntf::Bitcast(vtblAddrType, vtblAsVoidPtrPtr, vtblObject, voidPtrPtr));
    Cm::IrIntf::Assign(*emitter, voidPtrPtr, vtblAsVoidPtrPtr, vptr);
    result->Merge(thisResult);
    ResultStack().Push(result);
}

void CFunctionEmitter::RegisterDestructor(Cm::Sym::MemberVariableSymbol* staticMemberVariableSymbol)
{
    Cm::Core::Emitter* emitter = Emitter();
    Ir::Intf::Object* irObject = StaticMemberVariableRepository().GetStaticMemberVariableIrObject(staticMemberVariableSymbol);
    Ir::Intf::Object* destructionNode = StaticMemberVariableRepository().GetDestructionNode(staticMemberVariableSymbol);
    Ir::Intf::Type* voidPtr = Cm::IrIntf::Pointer(Ir::Intf::GetFactory()->GetVoid(), 1);
    emitter->Own(voidPtr);
    Ir::Intf::MemberVar* objectField = Cm::IrIntf::CreateMemberVar("cls", destructionNode, 1, voidPtr);
    objectField->SetDotMember();
    emitter->Own(objectField);
    emitter->Emit(Cm::IrIntf::Store(voidPtr, irObject, objectField, Ir::Intf::Indirection::addr, Ir::Intf::Indirection::none));
    Cm::Sym::TypeSymbol* type = staticMemberVariableSymbol->GetType();
    if (type->IsClassTypeSymbol())
    {
        Cm::Sym::ClassTypeSymbol* classType = static_cast<Cm::Sym::ClassTypeSymbol*>(type);
        if (classType->Destructor())
        {
            Cm::Sym::FunctionSymbol* destructor = classType->Destructor();
            Ir::Intf::Function* destructorIrFun = IrFunctionRepository().CreateIrFunction(destructor);
            Ir::Intf::Type* destructorPtrType = IrFunctionRepository().GetFunPtrIrType(destructor);
            std::vector<Ir::Intf::Type*> dtorParamTypes1(1, voidPtr->Clone());
            Ir::Intf::Type* destructorFieldType = Cm::IrIntf::Pointer(Cm::IrIntf::CreateFunctionType(Cm::IrIntf::Void(), dtorParamTypes1), 1);
            emitter->Own(destructorFieldType);
            std::vector<Ir::Intf::Type*> dtorParamTypes2(1, voidPtr->Clone());
            Ir::Intf::Type* destructorFieldPtrType = Cm::IrIntf::Pointer(Cm::IrIntf::CreateFunctionType(Cm::IrIntf::Void(), dtorParamTypes2), 2);
            emitter->Own(destructorFieldPtrType);
            Ir::Intf::MemberVar* destructorField = Cm::IrIntf::CreateMemberVar("destructor", destructionNode, 2, destructorFieldPtrType);
            destructorField->SetDotMember();
            emitter->Own(destructorField);
            Ir::Intf::RegVar* destructorPtr = Cm::IrIntf::CreateTemporaryRegVar(destructorPtrType);
            emitter->Own(destructorPtr);
            Ir::Intf::Object* destructorIrObject = Cm::IrIntf::CreateGlobal(destructorIrFun->Name(), destructorPtrType);
            emitter->Own(destructorIrObject);
            emitter->Emit(Cm::IrIntf::Load(destructorPtrType, destructorPtr, destructorIrObject, Ir::Intf::Indirection::none, Ir::Intf::Indirection::addr));
            Ir::Intf::Object* destructorFunPtr = Cm::IrIntf::CreateTemporaryRegVar(destructorFieldType);
            emitter->Own(destructorFunPtr);
            emitter->Emit(Cm::IrIntf::Bitcast(destructorPtrType, destructorFunPtr, destructorPtr, destructorFieldType));
            emitter->Emit(Cm::IrIntf::Store(destructorFieldPtrType, destructorFunPtr, destructorField, Ir::Intf::Indirection::none, Ir::Intf::Indirection::none));
            std::vector<Ir::Intf::Parameter*> registerFunParams;
            Ir::Intf::Parameter* param = Cm::IrIntf::CreateParameter("node", destructionNode->GetType()->Clone());
            emitter->Own(param);
            registerFunParams.push_back(param);
            Ir::Intf::Function* registerFun = Cm::IrIntf::CreateFunction(Cm::IrIntf::GetRegisterDestructorFunctionName(), Ir::Intf::GetFactory()->GetVoid(), registerFunParams);
            emitter->Own(registerFun);
            std::vector<Ir::Intf::Object*> registerFunArgs;
            registerFunArgs.push_back(destructionNode);
            Ir::Intf::RegVar* result = Cm::IrIntf::CreateTemporaryRegVar(Ir::Intf::GetFactory()->GetVoid());
            emitter->Own(result);
            emitter->Emit(Cm::IrIntf::Call(result, registerFun, registerFunArgs));
        }
    }
}

void CFunctionEmitter::GenVirtualCall(Cm::Sym::FunctionSymbol* fun, Cm::Core::GenResult& memberFunctionResult)
{
    Cm::Core::Emitter* emitter = Emitter();
    Ir::Intf::Object* objectPtr = memberFunctionResult.Arg1();
    Cm::Sym::ClassTypeSymbol* classType = fun->Class();
    Ir::Intf::Type* voidPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1);
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2);
    emitter->Own(voidPtrPtr);
    Ir::Intf::Object* vptrContainerPtr = objectPtr;
    int16_t vptrIndex = classType->VPtrIndex();
    if (vptrIndex == -1)
    {
        Cm::Sym::ClassTypeSymbol* vptrContainingType = classType->VPtrContainerClass();
        vptrIndex = vptrContainingType->VPtrIndex();
        Ir::Intf::Type* vptrContainingPtrIrType = Cm::IrIntf::Pointer(vptrContainingType->GetIrType(), 1);
        emitter->Own(vptrContainingPtrIrType);
        Ir::Intf::RegVar* containerPtr = Cm::IrIntf::CreateTemporaryRegVar(vptrContainingPtrIrType);
        emitter->Own(containerPtr);
        Ir::Intf::Type* classTypePtrIrType = Cm::IrIntf::Pointer(classType->GetIrType(), 1);
        emitter->Own(classTypePtrIrType);
        emitter->Emit(Cm::IrIntf::Bitcast(classTypePtrIrType, containerPtr, objectPtr, vptrContainingPtrIrType));
        vptrContainerPtr = containerPtr;
    }
    Ir::Intf::MemberVar* vptr = Cm::IrIntf::CreateMemberVar(Cm::IrIntf::GetVPtrVarName(), vptrContainerPtr, vptrIndex, voidPtrPtr);
    emitter->Own(vptr);
    Ir::Intf::RegVar* loadedVptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(loadedVptr);
    Cm::IrIntf::Assign(*emitter, voidPtrPtr, vptr, loadedVptr);
    Ir::Intf::RegVar* functionVoidPtrPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(functionVoidPtrPtr);
    Ir::Intf::Object* functionIndex = Cm::IrIntf::CreateI16Constant(fun->VtblIndex());
    emitter->Own(functionIndex);
    emitter->Emit(Cm::IrIntf::Add(voidPtrPtr, functionVoidPtrPtr, loadedVptr, functionIndex));
    Ir::Intf::RegVar* loadedFunctionVoidPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(loadedFunctionVoidPtr);
    Cm::IrIntf::Assign(*emitter, voidPtr, functionVoidPtrPtr, loadedFunctionVoidPtr);
    Ir::Intf::Type* functionPtrType = IrFunctionRepository().GetFunPtrIrType(fun);
    Ir::Intf::RegVar* loadedFunctionPtr = Cm::IrIntf::CreateTemporaryRegVar(functionPtrType);
    emitter->Own(loadedFunctionPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, loadedFunctionPtr, loadedFunctionVoidPtr, functionPtrType));
    Ir::Intf::Instruction* callInst = Cm::IrIntf::IndirectCall(memberFunctionResult.MainObject(), loadedFunctionPtr, memberFunctionResult.Args());
    if (GenerateDebugInfo())
    {
        Cm::Core::CfgNode* activeCfgNode = emitter->GetActiveCfgNode();
        if (activeCfgNode)
        {
            std::vector<std::string> funNames;
            funNames.push_back(IrFunctionRepository().CreateIrFunction(fun)->Name());
            for (Cm::Sym::FunctionSymbol* overrideFun : fun->OverrideSet())
            {
                funNames.push_back(IrFunctionRepository().CreateIrFunction(overrideFun)->Name());
            }
            Cm::Core::CFunCall* cFunCall = new Cm::Core::CFunCall(funNames);
            activeCfgNode->AddCFunCall(cFunCall);
            callInst->SetFunCallNode(cFunCall);
        }
    }
    emitter->Emit(callInst);
}

void CFunctionEmitter::GenInterfaceCall(Cm::Sym::FunctionSymbol* fun, Cm::Core::GenResult& memberFunctionResult)
{
    Cm::Core::Emitter* emitter = Emitter();
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(emitter, GenFlags()));
    Cm::Sym::Symbol* funParent = fun->Parent();
    if (!funParent->IsInterfaceTypeSymbol())
    {
        throw std::runtime_error("interface type expected");
    }
    Cm::Sym::InterfaceTypeSymbol* intf = static_cast<Cm::Sym::InterfaceTypeSymbol*>(funParent);
    Ir::Intf::Type* voidPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 1);
    emitter->Own(voidPtr);
    Ir::Intf::Type* voidPtrPtr = Cm::IrIntf::Pointer(Cm::IrIntf::Void(), 2);
    emitter->Own(voidPtrPtr);
    Ir::Intf::Object* mainObject = memberFunctionResult.Arg1();
    Ir::Intf::Type* intfPtrType = Cm::IrIntf::Pointer(intf->GetIrType(), 1);
    emitter->Own(intfPtrType);
    Ir::Intf::Object* intfObject = Cm::IrIntf::CreateTemporaryRegVar(intfPtrType);
    emitter->Own(intfObject);
    emitter->Emit(Cm::IrIntf::Load(voidPtr, intfObject, mainObject, Ir::Intf::Indirection::none, Ir::Intf::Indirection::none));
    Ir::Intf::MemberVar* obj = Cm::IrIntf::CreateMemberVar("obj", intfObject, 0, voidPtr);
    emitter->Own(obj);
    Ir::Intf::Object* objRegVar = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(objRegVar);
    Cm::IrIntf::Assign(*emitter, voidPtr, obj, objRegVar);
    int n = int(memberFunctionResult.Objects().size());
    for (int i = 0; i < n; ++i)
    {
        if (i == 1)
        {
            result->AddObject(objRegVar);
        }
        else
        {
            result->AddObject(memberFunctionResult.Objects()[i]);
        }
    }
    Ir::Intf::MemberVar* itab = Cm::IrIntf::CreateMemberVar("itab", intfObject, 1, voidPtr);
    emitter->Own(itab);
    Ir::Intf::Object* loadedItab = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(loadedItab);
    Cm::IrIntf::Assign(*emitter, voidPtr, itab, loadedItab);
    Ir::Intf::Object* itabPtrPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(itabPtrPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, itabPtrPtr, loadedItab, voidPtrPtr));
    Ir::Intf::Object* funIndex = Cm::IrIntf::CreateI32Constant(fun->ItblIndex());
    emitter->Own(funIndex);
    Ir::Intf::Object* funI8PtrPtr = Cm::IrIntf::CreateTemporaryRegVar(voidPtrPtr);
    emitter->Own(funI8PtrPtr);
    Ir::Intf::Object* zero = Cm::IrIntf::CreateUI32Constant(0);
    emitter->Own(zero);
    emitter->Emit(Cm::IrIntf::Add(voidPtrPtr, funI8PtrPtr, itabPtrPtr, funIndex));
    Ir::Intf::Object* funI8Ptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(funI8Ptr);
    Ir::Intf::RegVar* loadedFunI8Ptr = Cm::IrIntf::CreateTemporaryRegVar(voidPtr);
    emitter->Own(loadedFunI8Ptr);
    Cm::IrIntf::Assign(*emitter, voidPtr, funI8PtrPtr, loadedFunI8Ptr);
    Ir::Intf::Type* funPtrType = IrFunctionRepository().GetFunPtrIrType(fun);
    Ir::Intf::Object* funPtr = Cm::IrIntf::CreateTemporaryRegVar(funPtrType);
    emitter->Own(funPtr);
    emitter->Emit(Cm::IrIntf::Bitcast(voidPtr, funPtr, loadedFunI8Ptr, funPtrType));
    emitter->Emit(Cm::IrIntf::IndirectCall(result->MainObject(), funPtr, result->Args()));
    ResultStack().Push(result);
}

void CFunctionEmitter::SetCallDebugInfoInfo(Ir::Intf::Instruction* callInst, Ir::Intf::Function* fun)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::CfgNode* activeCfgNode = Emitter()->GetActiveCfgNode();
        if (activeCfgNode)
        {
            std::vector<std::string> funNames;
            funNames.push_back(fun->Name());
            Cm::Core::CFunCall* cFunCall = new Cm::Core::CFunCall(funNames);
            activeCfgNode->AddCFunCall(cFunCall);
            callInst->SetFunCallNode(cFunCall);
        }
    }
}

Ir::Intf::LabelObject* CFunctionEmitter::CreateLandingPadLabel(int landingPadId)
{
    return Cm::IrIntf::CreateLabel("_P_" + std::to_string(landingPadId));
}

void CFunctionEmitter::MapIrFunToFun(Ir::Intf::Function* irFun, Cm::Sym::FunctionSymbol* fun)
{
    (*functionMap)[irFun] = fun;
}

Ir::Intf::Object* CFunctionEmitter::MakeLocalVarIrObject(Cm::Sym::TypeSymbol* type, Ir::Intf::Object* source)
{
    if (type->IsClassTypeSymbol())
    {
        Ir::Intf::Object* target = Cm::IrIntf::CreateTemporaryRegVar(Cm::IrIntf::Pointer(type->GetIrType(), 1));
        Cm::Core::Emitter* emitter = Emitter();
        emitter->Own(target);
        Cm::IrIntf::Init(*emitter, type->GetIrType(), source, target);
        return target;
    }
    return source;
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundBeginThrowStatement& boundBeginThrowStatement)
{
    FunctionEmitter::Visit(boundBeginThrowStatement);
    if (GenerateDebugInfo())
    {
        if (!boundBeginThrowStatement.SyntaxNode())
        {
            throw std::runtime_error("no syntax node");
        }
        CreateDebugNode(boundBeginThrowStatement, boundBeginThrowStatement.SyntaxNode()->GetSpan(), false);
        boundBeginThrowStatement.GetCfgNode()->SetKind(Cm::Core::CfgNodeKind::throwNode);
    }
    PushGenDebugInfo(false);
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundEndThrowStatement& boundEndThrowStatement)
{
    PopGenDebugInfo();
    if (GenerateDebugInfo())
    {
        if (!boundEndThrowStatement.SyntaxNode())
        {
            throw std::runtime_error("no syntax node");
        }
        CreateDebugNode(boundEndThrowStatement, boundEndThrowStatement.SyntaxNode()->GetSpan(), false);
    }
    FunctionEmitter::Visit(boundEndThrowStatement);
}

void CFunctionEmitter::Visit(Cm::BoundTree::BoundBeginCatchStatement& boundBeginCatchStatement)
{
    PopGenDebugInfo();
    if (GenerateDebugInfo())
    {
        if (!boundBeginCatchStatement.SyntaxNode())
        {
            throw std::runtime_error("no syntax node");
        }
        CreateDebugNode(boundBeginCatchStatement, boundBeginCatchStatement.SyntaxNode()->GetSpan(), true);
        boundBeginCatchStatement.GetCfgNode()->SetKind(Cm::Core::CfgNodeKind::catchNode);
    }
    std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(Emitter(), GenFlags()));
    DoNothing(*result);
    ResultStack().Push(result);
}

Cm::Core::CfgNode* CFunctionEmitter::CreateDebugNode(Cm::BoundTree::BoundStatement& statement, const Cm::Parsing::Span& span, bool addToPrevNodes)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::ControlFlowGraph& cfg = functionDebugInfo->Cfg();
        Cm::Core::CfgNode* cfgNode = cfg.CreateNode(span, start, end);
        statement.SetCfgNode(cfgNode);
        cfg.PatchPrevNodes(cfgNode);
        Emitter()->UseCDebugNode(cfgNode);
        Emitter()->SetActiveCfgNode(cfgNode);
        if (addToPrevNodes)
        {
            cfg.AddToPrevNodes(cfgNode);
        }
        return cfgNode;
    }
    return nullptr;
}

void CFunctionEmitter::CreateDebugNode(Cm::BoundTree::BoundExpression& expr, const Cm::Parsing::Span& span)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::ControlFlowGraph& cfg = functionDebugInfo->Cfg();
        Cm::Core::CfgNode* cfgNode = cfg.CreateNode(span, start, end);
        expr.SetCfgNode(cfgNode);
        cfg.PatchPrevNodes(cfgNode);
        Emitter()->UseCDebugNode(cfgNode);
        Emitter()->SetActiveCfgNode(cfgNode);
    }
}

void CFunctionEmitter::AddDebugNodeTransition(Cm::BoundTree::BoundStatement& fromStatement, Cm::BoundTree::BoundStatement& toStatement)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::CfgNode* fromCfgNode = fromStatement.GetCfgNode();
        if (!fromCfgNode)
        {
            throw std::runtime_error("from cfg node not set");
        }
        Cm::Core::CfgNode* toCfgNode = toStatement.GetCfgNode();
        if (!toCfgNode)
        {
            throw std::runtime_error("to cfg node not set");
        }
        fromCfgNode->AddNext(toCfgNode->Id());
    }
}

void CFunctionEmitter::AddDebugNodeTransition(Cm::BoundTree::BoundExpression& fromExpression, Cm::BoundTree::BoundStatement& toStatement)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::CfgNode* fromCfgNode = fromExpression.GetCfgNode();
        if (!fromCfgNode)
        {
            throw std::runtime_error("from cfg node not set");
        }
        Cm::Core::CfgNode* toCfgNode = toStatement.GetCfgNode();
        if (!toCfgNode)
        {
            throw std::runtime_error("to cfg node not set");
        }
        fromCfgNode->AddNext(toCfgNode->Id());
    }
}

void CFunctionEmitter::AddDebugNodeTransition(Cm::BoundTree::BoundExpression& fromExpression, Cm::BoundTree::BoundExpression& toExpression)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::CfgNode* fromCfgNode = fromExpression.GetCfgNode();
        if (!fromCfgNode)
        {
            throw std::runtime_error("from cfg node not set");
        }
        Cm::Core::CfgNode* toCfgNode = toExpression.GetCfgNode();
        if (!toCfgNode)
        {
            throw std::runtime_error("to cfg node not set");
        }
        fromCfgNode->AddNext(toCfgNode->Id());
    }
}

int CFunctionEmitter::RetrievePrevDebugNodes()
{
    if (GenerateDebugInfo())
    {
        int debugNodeSetHandle = int(debugNodeSets.size());
        debugNodeSets.push_back(std::move(functionDebugInfo->Cfg().RetrievePrevNodes()));
        return debugNodeSetHandle;
    }
    return -1;
}

void CFunctionEmitter::AddToPrevDebugNodes(int debugNodeSetHandle)
{
    if (GenerateDebugInfo())
    {
        if (debugNodeSetHandle < 0 || debugNodeSetHandle >= int(debugNodeSets.size()))
        {
            throw std::runtime_error("invalid debug node set handle");
        }
        std::unordered_set<Cm::Core::CfgNode*> prevDebugNodes = std::move(debugNodeSets[debugNodeSetHandle]);
        functionDebugInfo->Cfg().AddToPrevNodes(prevDebugNodes);
    }
}

void CFunctionEmitter::AddToPrevDebugNodes(Cm::BoundTree::BoundStatement& statement)
{
    if (GenerateDebugInfo())
    {
        functionDebugInfo->Cfg().AddToPrevNodes(statement.GetCfgNode());
    }
}

void CFunctionEmitter::AddToPrevDebugNodes(Cm::BoundTree::BoundExpression& expr)
{
    if (GenerateDebugInfo())
    {
        functionDebugInfo->Cfg().AddToPrevNodes(expr.GetCfgNode());
    }
}

void CFunctionEmitter::AddToPrevDebugNodes(const std::unordered_set<Cm::Core::CfgNode*>& nodeSet)
{
    if (GenerateDebugInfo())
    {
        functionDebugInfo->Cfg().AddToPrevNodes(nodeSet);
    }
}

void CFunctionEmitter::CreateEntryDebugNode(Cm::BoundTree::BoundStatement& statement, const Cm::Parsing::Span& span)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::ControlFlowGraph& cfg = functionDebugInfo->Cfg();
        Cm::Core::CfgNode* cfgNode = cfg.CreateNode(span, start, end);
        cfg.PatchPrevNodes(cfgNode);
        statement.SetCfgNode(cfgNode);
        Emitter()->UseCDebugNode(cfgNode);
        std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(Emitter(), GenFlags()));
        Ir::Intf::LabelObject* entryLabel = Cm::IrIntf::CreateNextLocalLabel();
        Emitter()->Own(entryLabel);
        Emitter()->AddNextInstructionLabel(entryLabel);
        DoNothing(*result);
        cfg.AddToPrevNodes(cfgNode);
        Emitter()->SetActiveCfgNode(cfgNode);
        ResultStack().Push(result);
    }
}

void CFunctionEmitter::CreateExitDebugNode(Cm::BoundTree::BoundStatement& statement, const Cm::Parsing::Span& span)
{
    if (GenerateDebugInfo())
    {
        Cm::Core::ControlFlowGraph& cfg = functionDebugInfo->Cfg();
        Cm::Core::CfgNode* cfgNode = cfg.CreateNode(span, start, end);
        cfg.PatchPrevNodes(cfgNode);
        Emitter()->UseCDebugNode(cfgNode);
        std::shared_ptr<Cm::Core::GenResult> result(new Cm::Core::GenResult(Emitter(), GenFlags()));
        Ir::Intf::LabelObject* exitLabel = Cm::IrIntf::CreateNextLocalLabel();
        Emitter()->Own(exitLabel);
        Emitter()->AddNextInstructionLabel(exitLabel);
        DoNothing(*result);
        CompoundResult()->BackpatchNextTargets(result->GetLabel());
        cfg.AddToPrevNodes(cfgNode);
        Emitter()->SetActiveCfgNode(cfgNode);
        ResultStack().Push(result);
    }
}

void CFunctionEmitter::PatchPrevDebugNodes(Cm::BoundTree::BoundStatement& statement)
{
    if (GenerateDebugInfo())
    {
        functionDebugInfo->Cfg().PatchPrevNodes(statement.GetCfgNode());
    }
}

void CFunctionEmitter::SetCfgNode(Cm::BoundTree::BoundStatement& fromStatement, Cm::BoundTree::BoundStatement& toStatement)
{
    if (GenerateDebugInfo())
    {
        toStatement.SetCfgNode(fromStatement.GetCfgNode());
    }
}

void CFunctionEmitter::PatchDebugNodes(const std::unordered_set<Cm::Core::CfgNode*>& nodeSet, Cm::Core::CfgNode* nextNode)
{
    if (GenerateDebugInfo())
    {
        functionDebugInfo->Cfg().Patch(nodeSet, nextNode);
    }
}

} } // namespace Cm::Emit
