/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#include <Cm.Bind/StatementBinder.hpp>
#include <Cm.Bind/LocalVariable.hpp>
#include <Cm.Bind/OverloadResolution.hpp>
#include <Cm.Bind/TypeResolver.hpp>
#include <Cm.Bind/Evaluator.hpp>
#include <Cm.Bind/Function.hpp>
#include <Cm.Bind/Access.hpp>
#include <Cm.Bind/Binder.hpp>
#include <Cm.Core/Exception.hpp>
#include <Cm.Sym/DeclarationVisitor.hpp>
#include <Cm.Sym/ExceptionTable.hpp>
#include <Cm.Parser/FileRegistry.hpp>
#include <Cm.Ast/Identifier.hpp>
#include <Cm.Ast/Expression.hpp>
#include <Cm.Ast/Literal.hpp>
#include <Cm.Ast/BasicType.hpp>
#include <Cm.IrIntf/Rep.hpp>

namespace Cm { namespace Bind {

StatementBinder::StatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_) :
    ExpressionBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), symbolTable(boundCompileUnit_.SymbolTable()), containerScope(containerScope_), fileScopes(fileScopes_), 
    result(nullptr)
{
}

ConstructionStatementBinder::ConstructionStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), constructionStatement(nullptr)
{
}

void ConstructionStatementBinder::BeginVisit(Cm::Ast::ConstructionStatementNode& constructionStatementNode)
{
    constructionStatement = new Cm::BoundTree::BoundConstructionStatement(&constructionStatementNode);
    Cm::Sym::LocalVariableSymbol* localVariable = BindLocalVariable(SymbolTable(), ContainerScope(), FileScopes(), BoundCompileUnit().ClassTemplateRepository(), &constructionStatementNode);
    constructionStatement->SetLocalVariable(localVariable);
    CurrentFunction()->AddLocalVariable(localVariable);
}

void ConstructionStatementBinder::EndVisit(Cm::Ast::ConstructionStatementNode& constructionStatementNode)
{
    if (constructionStatement->LocalVariable()->Used())
    { 
        throw Cm::Core::Exception("local variable '" + constructionStatement->LocalVariable()->Name() + "' is used before it is defined", constructionStatement->LocalVariable()->GetUseSpan(), 
            constructionStatementNode.GetSpan());
    }
    constructionStatement->SetArguments(GetExpressions());
    std::vector<Cm::Core::Argument> resolutionArguments;
    Cm::Sym::TypeSymbol* localVariableType = constructionStatement->LocalVariable()->GetType();
    if (localVariableType->IsAbstract())
    {
        throw Cm::Core::Exception("cannot instantiate an abstract class", constructionStatementNode.GetSpan());
    }
    Cm::Core::Argument variableArgument(Cm::Core::ArgumentCategory::lvalue, SymbolTable().GetTypeRepository().MakePointerType(localVariableType, constructionStatementNode.GetSpan()));
    resolutionArguments.push_back(variableArgument);
    constructionStatement->GetResolutionArguments(localVariableType, resolutionArguments);
    Cm::Sym::FunctionLookupSet functionLookups;
    functionLookups.Add(Cm::Sym::FunctionLookup(Cm::Sym::ScopeLookup::this_, constructionStatement->LocalVariable()->GetType()->GetContainerScope()->ClassOrNsScope()));
    std::vector<Cm::Sym::FunctionSymbol*> conversions;
    Cm::Sym::ConversionType conversionType = Cm::Sym::ConversionType::implicit;
    Cm::Sym::FunctionSymbol* ctor = ResolveOverload(ContainerScope(), BoundCompileUnit(), "@constructor", resolutionArguments, functionLookups, constructionStatementNode.GetSpan(), conversions, 
        conversionType,  OverloadResolutionFlags::none);
    PrepareFunctionSymbol(ctor, constructionStatementNode.GetSpan());
    constructionStatement->SetConstructor(ctor);
    constructionStatement->InsertLocalVariableToArguments();
    constructionStatement->Arguments()[0]->SetFlag(Cm::BoundTree::BoundNodeFlags::constructVariable);
    constructionStatement->ApplyConversions(conversions, CurrentFunction());
    PrepareFunctionArguments(ctor, ContainerScope(), BoundCompileUnit(), CurrentFunction(), constructionStatement->Arguments(), true, BoundCompileUnit().IrClassTypeRepository());
    if (localVariableType->IsReferenceType())
    {
        constructionStatement->Arguments()[1]->SetFlag(Cm::BoundTree::BoundNodeFlags::argByRef);
    }
    SetResult(constructionStatement);
}

AssignmentStatementBinder::AssignmentStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void AssignmentStatementBinder::EndVisit(Cm::Ast::AssignmentStatementNode& assignmentStatementNode)
{
    Cm::BoundTree::BoundExpression* right = Pop();
    Cm::BoundTree::BoundExpression* left = Pop();
    left->SetFlag(Cm::BoundTree::BoundNodeFlags::lvalue);
    std::vector<Cm::Core::Argument> resolutionArguments;
    Cm::Sym::TypeSymbol* leftPlainType = SymbolTable().GetTypeRepository().MakePlainType(left->GetType());
    Cm::Core::Argument leftArgument(Cm::Core::ArgumentCategory::lvalue, SymbolTable().GetTypeRepository().MakePointerType(leftPlainType, assignmentStatementNode.GetSpan()));
    resolutionArguments.push_back(leftArgument);
    if (leftPlainType->IsPointerType() && right->IsBoundNullLiteral())
    {
        right->SetType(leftPlainType);
    }
    Cm::Core::Argument rightArgument = Cm::Core::Argument(right->GetArgumentCategory(), right->GetType());
    if (right->GetFlag(Cm::BoundTree::BoundNodeFlags::argIsTemporary))
    {
        rightArgument.SetBindToRvalueRef();
    }
    resolutionArguments.push_back(rightArgument);
    Cm::Sym::FunctionLookupSet functionLookups;
    functionLookups.Add(Cm::Sym::FunctionLookup(Cm::Sym::ScopeLookup::this_, leftPlainType->GetContainerScope()->ClassOrNsScope()));
    std::vector<Cm::Sym::FunctionSymbol*> conversions;
    Cm::Sym::FunctionSymbol* assignment = ResolveOverload(ContainerScope(), BoundCompileUnit(), "operator=", resolutionArguments, functionLookups, assignmentStatementNode.GetSpan(), conversions);
    PrepareFunctionSymbol(assignment, assignmentStatementNode.GetSpan());
    if (conversions.size() != 2)
    {
        throw std::runtime_error("wrong number of conversions");
    }
    Cm::Sym::FunctionSymbol* leftConversion = conversions[0];
    if (leftConversion)
    {
        left = Cm::BoundTree::CreateBoundConversion(&assignmentStatementNode, left, leftConversion, CurrentFunction());
    }
    Cm::Sym::FunctionSymbol* rightConversion = conversions[1];
    if (rightConversion)
    {
        right = Cm::BoundTree::CreateBoundConversion(&assignmentStatementNode, right, rightConversion, CurrentFunction());
    }
    if (!assignment->Parameters()[1]->GetType()->IsReferenceType() && right->GetType()->IsReferenceType())
    {
        right->SetFlag(Cm::BoundTree::BoundNodeFlags::refByValue);
    }
    Cm::BoundTree::BoundAssignmentStatement* assignmentStatement = new Cm::BoundTree::BoundAssignmentStatement(&assignmentStatementNode, left, right, assignment);
    SetResult(assignmentStatement);
}

SimpleStatementBinder::SimpleStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void SimpleStatementBinder::EndVisit(Cm::Ast::SimpleStatementNode& simpleStatementNode)
{
    Cm::BoundTree::BoundSimpleStatement* simpleStatement = new Cm::BoundTree::BoundSimpleStatement(&simpleStatementNode);
    if (simpleStatementNode.HasExpr())
    {
        Cm::BoundTree::BoundExpression* expression = Pop();
        simpleStatement->SetExpression(expression);
    }
    SetResult(simpleStatement);
}

ReturnStatementBinder::ReturnStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void ReturnStatementBinder::EndVisit(Cm::Ast::ReturnStatementNode& returnStatementNode)
{
    Cm::BoundTree::BoundReturnStatement* returnStatement = new Cm::BoundTree::BoundReturnStatement(&returnStatementNode);
    Cm::Ast::FunctionNode* functionNode = returnStatementNode.GetFunction();
    Cm::Ast::Node* returnTypeExpr = functionNode->ReturnTypeExpr();
    if (returnTypeExpr)
    {
        Cm::Sym::TypeSymbol* returnType = ResolveType(SymbolTable(), ContainerScope(), FileScopes(), BoundCompileUnit().ClassTemplateRepository(), returnTypeExpr);
        if (!returnType->IsVoidTypeSymbol())
        {
            if (returnStatementNode.ReturnsValue())
            {
                returnStatement->SetReturnType(returnType);
                Cm::BoundTree::BoundExpression* returnValue = Pop();
                std::vector<Cm::Core::Argument> resolutionArguments;
                Cm::Core::Argument targetArgument(Cm::Core::ArgumentCategory::lvalue, SymbolTable().GetTypeRepository().MakePointerType(returnType, returnStatementNode.GetSpan()));
                resolutionArguments.push_back(targetArgument);
                if (returnType->IsNonConstReferenceType() && (returnValue->GetType()->IsNonConstReferenceType() || !returnValue->GetType()->IsReferenceType()))
                {
                    Cm::Core::Argument sourceArgument = Cm::Core::Argument(returnValue->GetArgumentCategory(), SymbolTable().GetTypeRepository().MakeReferenceType(returnValue->GetType(),
                        returnStatementNode.GetSpan()));
                    resolutionArguments.push_back(sourceArgument);
                    if (!returnValue->GetType()->IsReferenceType())
                    {
                        returnValue->SetFlag(Cm::BoundTree::BoundNodeFlags::lvalue);
                    }
                }
                else
                {
                    Cm::Core::Argument sourceArgument = Cm::Core::Argument(returnValue->GetArgumentCategory(), SymbolTable().GetTypeRepository().MakeConstReferenceType(returnValue->GetType(),
                        returnStatementNode.GetSpan()));
                    resolutionArguments.push_back(sourceArgument);
                    if (returnType->IsReferenceType())
                    {
                        if (!returnValue->GetType()->IsReferenceType())
                        {
                            returnValue->SetFlag(Cm::BoundTree::BoundNodeFlags::lvalue);
                        }
                    }
                }
                if (returnValue->GetFlag(Cm::BoundTree::BoundNodeFlags::argIsTemporary) || returnValue->IsBoundLocalVariable())
                {
                    Cm::Core::Argument& sourceArgument = resolutionArguments[1];
                    sourceArgument.SetBindToRvalueRef();
                }
                Cm::Sym::FunctionLookupSet functionLookups;
                functionLookups.Add(Cm::Sym::FunctionLookup(Cm::Sym::ScopeLookup::this_, returnType->GetContainerScope()->ClassOrNsScope()));
                std::vector<Cm::Sym::FunctionSymbol*> conversions;
                Cm::Sym::ConversionType conversionType = Cm::Sym::ConversionType::implicit;
                if (returnValue->IsCast())
                {
                    conversionType = Cm::Sym::ConversionType::explicit_;
                }
                Cm::Sym::FunctionSymbol* ctor = ResolveOverload(ContainerScope(), BoundCompileUnit(), "@constructor", resolutionArguments, functionLookups, returnStatementNode.GetSpan(), conversions, 
                    conversionType, OverloadResolutionFlags::none);
                PrepareFunctionSymbol(ctor, returnStatementNode.GetSpan());
                returnStatement->SetConstructor(ctor);
                if (conversions.size() != 2)
                {
                    throw std::runtime_error("wrong number of conversions");
                }
                Cm::Sym::FunctionSymbol* conversionFun = conversions[1];
                if (conversionFun)
                {
                    returnValue = Cm::BoundTree::CreateBoundConversion(&returnStatementNode, returnValue, conversionFun, CurrentFunction());
                }
                if (returnValue->GetType()->IsClassTypeSymbol() && returnType->IsReferenceType())
                {
                    returnValue->SetFlag(Cm::BoundTree::BoundNodeFlags::argByRef);
                }
                returnStatement->SetExpression(returnValue);
            }
            else
            {
                throw Cm::Core::Exception("must return a value", returnStatementNode.GetSpan(), functionNode->GetSpan());
            }
        }
        else if (returnStatementNode.ReturnsValue())
        {
            throw Cm::Core::Exception("void function cannot return a value", returnStatementNode.GetSpan(), functionNode->GetSpan());
        }
    }
    else if (returnStatementNode.ReturnsValue())
    {
        throw Cm::Core::Exception("cannot return a value", returnStatementNode.GetSpan(), functionNode->GetSpan());
    }
    SetResult(returnStatement);
}

ConditionalStatementBinder::ConditionalStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundConditionalStatement* conditionalStatement_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), conditionalStatement(conditionalStatement_)
{
    PushSkipContent();
}

void ConditionalStatementBinder::EndVisit(Cm::Ast::ConditionalStatementNode& conditionalStatementNode)
{
    PopSkipContent();
    Cm::BoundTree::BoundExpression* condition = Pop();
    Cm::Sym::TypeSymbol* condType = SymbolTable().GetTypeRepository().MakePlainType(condition->GetType());
    if (!condType->IsBoolTypeSymbol())
    {
        throw Cm::Core::Exception("if statement condition must be Boolean expression (now of type '" + condType->FullName() + "')", conditionalStatementNode.Condition()->GetSpan());
    }
    condition->SetFlag(Cm::BoundTree::BoundNodeFlags::genJumpingBoolCode);
    conditionalStatement->SetCondition(condition);
}

WhileStatementBinder::WhileStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundWhileStatement* whileStatement_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_,  currentFunction_), whileStatement(whileStatement_)
{
    PushSkipContent();
}

void WhileStatementBinder::EndVisit(Cm::Ast::WhileStatementNode& whileStatementNode) 
{
    PopSkipContent();
    Cm::BoundTree::BoundExpression* condition = Pop();
    Cm::Sym::TypeSymbol* condType = SymbolTable().GetTypeRepository().MakePlainType(condition->GetType());
    if (!condType->IsBoolTypeSymbol())
    {
        throw Cm::Core::Exception("while statement condition must be Boolean expression (now of type '" + condType->FullName() + "')", whileStatementNode.Condition()->GetSpan());
    }
    condition->SetFlag(Cm::BoundTree::BoundNodeFlags::genJumpingBoolCode);
    whileStatement->SetCondition(condition);
}

DoStatementBinder::DoStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundDoStatement* doStatement_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), doStatement(doStatement_)
{
    PushSkipContent();
}

void DoStatementBinder::EndVisit(Cm::Ast::DoStatementNode& doStatementNode)
{
    PopSkipContent();
    Cm::BoundTree::BoundExpression* condition = Pop();
    Cm::Sym::TypeSymbol* condType = SymbolTable().GetTypeRepository().MakePlainType(condition->GetType());
    if (!condType->IsBoolTypeSymbol())
    {
        throw Cm::Core::Exception("do statement condition must be Boolean expression (now of type '" + condType->FullName() + "')", doStatementNode.Condition()->GetSpan());
    }
    condition->SetFlag(Cm::BoundTree::BoundNodeFlags::genJumpingBoolCode);
    doStatement->SetCondition(condition);
}

ForStatementBinder::ForStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundForStatement* forStatement_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), forStatement(forStatement_)
{
    PushSkipContent();
}

void ForStatementBinder::EndVisit(Cm::Ast::ForStatementNode& forStatementNode)
{
    PopSkipContent();
    if (!forStatementNode.HasIncrement())
    {
        GenerateTrueExpression(&forStatementNode);
    }
    Cm::BoundTree::BoundExpression* increment = Pop();
    forStatement->SetIncrement(increment);
    if (!forStatementNode.HasCondition())
    {
        GenerateTrueExpression(&forStatementNode);
    }
    Cm::BoundTree::BoundExpression* condition = Pop();
    Cm::Sym::TypeSymbol* condType = SymbolTable().GetTypeRepository().MakePlainType(condition->GetType());
    if (!condType->IsBoolTypeSymbol())
    {
        throw Cm::Core::Exception("for statement condition must be Boolean expression (now of type '" + condType->FullName() + "')", forStatementNode.Condition()->GetSpan());
    }
    condition->SetFlag(Cm::BoundTree::BoundNodeFlags::genJumpingBoolCode);
    forStatement->SetCondition(condition);
}

RangeForStatementBinder::RangeForStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_,
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_, Cm::Ast::RangeForStatementNode& rangeForStatementNode, Binder& binder_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), binder(binder_)
{
}

void RangeForStatementBinder::EndVisit(Cm::Ast::RangeForStatementNode& rangeForStatementNode) 
{
    rangeForStatementNode.Container()->Accept(*this);
    std::unique_ptr<Cm::BoundTree::BoundExpression> container(Pop());
    Cm::Sym::TypeSymbol* containerType = container->GetType();
    Cm::Sym::TypeSymbol* plainContainerType = SymbolTable().GetTypeRepository().MakePlainType(containerType);
    Cm::Ast::IdentifierNode* beginNode(new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), "Begin"));
    Cm::Ast::IdentifierNode* endNode(new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), "End"));
    Cm::Ast::CloneContext cloneContext;
    Cm::Ast::DotNode* containerBeginNode(new Cm::Ast::DotNode(rangeForStatementNode.GetSpan(), rangeForStatementNode.Container()->Clone(cloneContext), beginNode));
    Cm::Ast::InvokeNode* invokeContainerBeginNode(new Cm::Ast::InvokeNode(rangeForStatementNode.GetSpan(), containerBeginNode));
    Cm::Ast::DotNode* containerEndNode(new Cm::Ast::DotNode(rangeForStatementNode.GetSpan(), rangeForStatementNode.Container()->Clone(cloneContext), endNode));
    Cm::Ast::InvokeNode* invokeContainerEndNode(new Cm::Ast::InvokeNode(rangeForStatementNode.GetSpan(), containerEndNode));
    Cm::Ast::IdentifierNode* iteratorId(new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), CurrentFunction()->GetNextTempVariableName()));
    Cm::Ast::IdentifierNode* iteratorTypeId = nullptr; 
    if (containerType->IsConstType())
    {
        iteratorTypeId = new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), "ConstIterator");
    }
    else
    {
        iteratorTypeId = new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), "Iterator");
    }
    Cm::Ast::IdentifierNode* containerTypeId(new Cm::Ast::IdentifierNode(rangeForStatementNode.GetSpan(), plainContainerType->FullName()));
    Cm::Ast::DotNode* containerIterator(new Cm::Ast::DotNode(rangeForStatementNode.GetSpan(), containerTypeId, iteratorTypeId));
    Cm::Ast::ConstructionStatementNode* initNode(new Cm::Ast::ConstructionStatementNode(rangeForStatementNode.GetSpan(), containerIterator, iteratorId));
    initNode->AddArgument(invokeContainerBeginNode);
    Cm::Ast::NotEqualNode* iteratorNotEqualToEnd(new Cm::Ast::NotEqualNode(rangeForStatementNode.GetSpan(), iteratorId->Clone(cloneContext), invokeContainerEndNode));
    Cm::Ast::PrefixIncNode* incIterator(new Cm::Ast::PrefixIncNode(rangeForStatementNode.GetSpan(), iteratorId->Clone(cloneContext)));
    Cm::Ast::CompoundStatementNode* action(new Cm::Ast::CompoundStatementNode(rangeForStatementNode.GetSpan()));
    Cm::Ast::DerefNode* derefIterator(new Cm::Ast::DerefNode(rangeForStatementNode.GetSpan(), iteratorId->Clone(cloneContext)));
    Cm::Ast::ConstructionStatementNode* initVariable(new Cm::Ast::ConstructionStatementNode(rangeForStatementNode.GetSpan(), rangeForStatementNode.VarTypeExpr()->Clone(cloneContext), 
        static_cast<Cm::Ast::IdentifierNode*>(rangeForStatementNode.VarId()->Clone(cloneContext))));
    initVariable->AddArgument(derefIterator);
    action->AddStatement(initVariable);
    Cm::Sym::DeclarationVisitor declarationVisitor(SymbolTable());
    action->AddStatement(static_cast<Cm::Ast::StatementNode*>(rangeForStatementNode.Action()->Clone(cloneContext)));
    Cm::Ast::ForStatementNode* forStatementNode = new Cm::Ast::ForStatementNode(rangeForStatementNode.GetSpan(), initNode, iteratorNotEqualToEnd, incIterator, action);
    CurrentFunction()->Own(forStatementNode);
    forStatementNode->Accept(declarationVisitor);
    Cm::Sym::ContainerScope* containerScope = SymbolTable().GetContainerScope(forStatementNode);
    containerScope->SetParent(ContainerScope());
    binder.BeginContainerScope(containerScope);
    forStatementNode->Accept(binder);
    binder.EndContainerScope();
}

SwitchStatementBinder::SwitchStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundSwitchStatement* switchStatement_): 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), switchStatement(switchStatement_)
{
}

void SwitchStatementBinder::BeginVisit(Cm::Ast::SwitchStatementNode& switchStatementNode)
{
    PushSkipContent();
    Cm::Ast::Node* conditionNode = switchStatementNode.Condition();
    conditionNode->Accept(*this);
    Cm::BoundTree::BoundExpression* condition = Pop();
    Cm::Sym::TypeSymbol* condType = SymbolTable().GetTypeRepository().MakePlainType(condition->GetType());
    if (condType->IsIntegerTypeSymbol() || condType->IsCharTypeSymbol() || condType->IsBoolTypeSymbol() || condType->IsEnumTypeSymbol())
    {
        switchStatement->SetCondition(condition);
    }
    else
    {
        throw Cm::Core::Exception("switch statement condition must be of integer, character, enumerated or Boolean type", conditionNode->GetSpan());
    }
}

void SwitchStatementBinder::EndVisit(Cm::Ast::SwitchStatementNode& switchStatementNode)
{
    PopSkipContent();
}

CaseStatementBinder::CaseStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundCaseStatement* caseStatement_, Cm::BoundTree::BoundSwitchStatement* switchStatement_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), caseStatement(caseStatement_), switchStatement(switchStatement_)
{
    PushSkipContent();
}

bool TerminatesCase(Cm::Ast::StatementNode* statementNode)
{
    if (statementNode->IsConditionalStatementNode())
    {
        Cm::Ast::ConditionalStatementNode* conditionalStatement = static_cast<Cm::Ast::ConditionalStatementNode*>(statementNode);
        if (conditionalStatement->HasElseStatement())
        {
            if (TerminatesCase(conditionalStatement->ThenS()) && TerminatesCase(conditionalStatement->ElseS()))
            {
                return true;
            }
        }
    }
    else if (statementNode->IsCompoundStatementNode())
    {
        Cm::Ast::CompoundStatementNode* compoundStatement = static_cast<Cm::Ast::CompoundStatementNode*>(statementNode);
        for (const std::unique_ptr<Cm::Ast::StatementNode>& statement : compoundStatement->Statements())
        {
            if (TerminatesCase(statement.get())) return true;
        }
    }
    else
    {
        return statementNode->IsCaseTerminatingNode();
    }
    return false;
}

void CaseStatementBinder::EndVisit(Cm::Ast::CaseStatementNode& caseStatementNode)
{
    PopSkipContent();
    for (const std::unique_ptr<Cm::Ast::Node>& expr : caseStatementNode.Expressions())
    {
        Cm::Sym::TypeSymbol* condType = switchStatement->Condition()->GetType();
        if (condType->IsEnumTypeSymbol())
        {
            Cm::Sym::EnumTypeSymbol* enumTypeSymbol = static_cast<Cm::Sym::EnumTypeSymbol*>(condType);
            condType = enumTypeSymbol->GetUnderlyingType();
        }
        Cm::Sym::SymbolType symbolType = condType->GetSymbolType();
        Cm::Sym::ValueType valueType = Cm::Sym::GetValueTypeFor(symbolType);
        Cm::Sym::Value* value = Evaluate(valueType, false, expr.get(), SymbolTable(), ContainerScope(), FileScopes(), BoundCompileUnit().ClassTemplateRepository());
        caseStatement->AddValue(value);
    }
    for (const std::unique_ptr<Cm::Ast::StatementNode>& statement : caseStatementNode.Statements())
    {
        if (TerminatesCase(statement.get())) return;
    }
    throw Cm::Core::Exception("case must end in break, continue, return, throw, goto, goto case or goto default statement", caseStatementNode.GetSpan());
}

DefaultStatementBinder::DefaultStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundDefaultStatement* defaultStatement_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_),
    defaultStatement(defaultStatement_)
{
}

void DefaultStatementBinder::EndVisit(Cm::Ast::DefaultStatementNode& defaultStatementNode)
{
    for (const std::unique_ptr<Cm::Ast::StatementNode>& statement : defaultStatementNode.Statements())
    {
        if (TerminatesCase(statement.get())) return;
    }
    throw Cm::Core::Exception("default case must end in break, continue, return, throw goto, goto case or goto default statement", defaultStatementNode.GetSpan());
}

BreakStatementBinder::BreakStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void BreakStatementBinder::Visit(Cm::Ast::BreakStatementNode& breakStatementNode)
{
    Cm::Ast::Node* parent = breakStatementNode.Parent();
    while (parent && !parent->IsBreakEnclosingStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Cm::Core::Exception("break statement must be enclosed in while, do, for or switch statement", breakStatementNode.GetSpan());
    }
    Cm::BoundTree::BoundBreakStatement* boundBreakStatement = new Cm::BoundTree::BoundBreakStatement(&breakStatementNode);
    SetResult(boundBreakStatement);
}

ContinueStatementBinder::ContinueStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void ContinueStatementBinder::Visit(Cm::Ast::ContinueStatementNode& continueStatementNode)
{
    Cm::Ast::Node* parent = continueStatementNode.Parent();
    while (parent && !parent->IsContinueEnclosingStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Cm::Core::Exception("continue statement must be enclosed in while, do, or for statement", continueStatementNode.GetSpan());
    }
    Cm::BoundTree::BoundContinueStatement* boundContinueStatement = new Cm::BoundTree::BoundContinueStatement(&continueStatementNode);
    SetResult(boundContinueStatement);
}

GotoCaseStatementBinder::GotoCaseStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_, Cm::BoundTree::BoundSwitchStatement* switchStatement_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), switchStatement(switchStatement_)
{
}

void GotoCaseStatementBinder::EndVisit(Cm::Ast::GotoCaseStatementNode& gotoCaseStatementNode)
{
    Cm::Ast::Node* parent = gotoCaseStatementNode.Parent();
    while (parent && !parent->IsCaseStatementNode() && !parent->IsDefaultStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Cm::Core::Exception("goto case statement must be enclosed in case or default statement", gotoCaseStatementNode.GetSpan());
    }
    Cm::Sym::TypeSymbol* condType = switchStatement->Condition()->GetType();
    if (condType->IsEnumTypeSymbol())
    {
        Cm::Sym::EnumTypeSymbol* enumTypeSymbol = static_cast<Cm::Sym::EnumTypeSymbol*>(condType);
        condType = enumTypeSymbol->GetUnderlyingType();
    }
    Cm::Sym::SymbolType symbolType = condType->GetSymbolType();
    Cm::Sym::ValueType valueType = Cm::Sym::GetValueTypeFor(symbolType);
    Cm::Sym::Value* value = Evaluate(valueType, false, gotoCaseStatementNode.TargetCaseExpr(), SymbolTable(), ContainerScope(), FileScopes(), BoundCompileUnit().ClassTemplateRepository());
    Cm::BoundTree::BoundGotoCaseStatement* boundGotoCasetatement = new Cm::BoundTree::BoundGotoCaseStatement(&gotoCaseStatementNode);
    boundGotoCasetatement->SetValue(value);
    SetResult(boundGotoCasetatement);
}

GotoDefaultStatementBinder::GotoDefaultStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, 
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : 
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void GotoDefaultStatementBinder::Visit(Cm::Ast::GotoDefaultStatementNode& gotoDefaultStatementNode)
{
    Cm::Ast::Node* parent = gotoDefaultStatementNode.Parent();
    while (parent && !parent->IsCaseStatementNode())
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Cm::Core::Exception("goto default statement must be enclosed in case statement", gotoDefaultStatementNode.GetSpan());
    }
    Cm::BoundTree::BoundGotoDefaultStatement* boundGotoDefaultStatement = new Cm::BoundTree::BoundGotoDefaultStatement(&gotoDefaultStatementNode);
    SetResult(boundGotoDefaultStatement);
}

DestroyStatementBinder::DestroyStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_,
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void DestroyStatementBinder::EndVisit(Cm::Ast::DestroyStatementNode& destroyStatementNode)
{
    std::unique_ptr<Cm::BoundTree::BoundExpression> ptr(Pop());
    Cm::Sym::TypeSymbol* type = ptr->GetType();
    if (!type->IsPointerType())
    {
        throw Cm::Core::Exception("destroy statement needs pointer type operand", destroyStatementNode.GetSpan());
    }
    if (type->GetPointerCount() == 1)
    {
        Cm::Sym::TypeSymbol* baseType = type->GetBaseType();
        if (baseType->IsClassTypeSymbol())
        {
            Cm::Sym::ClassTypeSymbol* classType = static_cast<Cm::Sym::ClassTypeSymbol*>(baseType);
            Cm::Sym::FunctionSymbol* destructor = classType->Destructor();
            if (destructor)
            {
                Cm::BoundTree::BoundExpressionList arguments;
                arguments.Add(ptr.release());
                Cm::BoundTree::BoundFunctionCallStatement* destructionStatement = new Cm::BoundTree::BoundFunctionCallStatement(destructor, std::move(arguments));
                SetResult(destructionStatement);
                return;
            }
        }
    }
    SetResult(nullptr);
}

DeleteStatementBinder::DeleteStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_,
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_)
{
}

void DeleteStatementBinder::EndVisit(Cm::Ast::DeleteStatementNode& deleteStatementNode)
{
    bool resultSet = false;
    std::unique_ptr<Cm::BoundTree::BoundExpression> ptr(Pop());
    Cm::Sym::TypeSymbol* type = ptr->GetType();
    if (!type->IsPointerType())
    {
        throw Cm::Core::Exception("delete statement needs pointer type operand", deleteStatementNode.GetSpan());
    }
    if (type->GetPointerCount() == 1)
    {
        Cm::Sym::TypeSymbol* baseType = type->GetBaseType();
        if (baseType->IsClassTypeSymbol())
        {
            Cm::Sym::ClassTypeSymbol* classType = static_cast<Cm::Sym::ClassTypeSymbol*>(baseType);
            Cm::Sym::FunctionSymbol* destructor = classType->Destructor();
            if (destructor)
            {
                Cm::BoundTree::BoundExpressionList arguments;
                arguments.Add(ptr.release());
                Cm::BoundTree::BoundFunctionCallStatement* destructionStatement = new Cm::BoundTree::BoundFunctionCallStatement(destructor, std::move(arguments));
                SetResult(destructionStatement);
                resultSet = true;
            }
        }
    }
    if (!resultSet)
    {
        SetResult(nullptr);
    }
    deleteStatementNode.PointerExpr()->Accept(*this);
    std::unique_ptr<Cm::BoundTree::BoundExpression> mem(Pop());
    std::vector<Cm::Core::Argument> resolutionArguments;
    resolutionArguments.push_back(Cm::Core::Argument(Cm::Core::ArgumentCategory::rvalue, mem->GetType()));
    Cm::Sym::FunctionLookupSet functionLookups;
    functionLookups.Add(Cm::Sym::FunctionLookup(Cm::Sym::ScopeLookup::this_and_parent, ContainerScope()));
    std::vector<Cm::Sym::FunctionSymbol*> conversions;
    Cm::Sym::FunctionSymbol* memFreeFun = ResolveOverload(ContainerScope(), BoundCompileUnit(), "System.Support.MemFree", resolutionArguments, functionLookups, deleteStatementNode.GetSpan(), conversions);
    if (conversions.size() != 1)
    {
        throw std::runtime_error("wrong number of conversions");
    }
    Cm::BoundTree::BoundExpressionList arguments;
    if (conversions[0])
    {
        arguments.Add(CreateBoundConversion(&deleteStatementNode, mem.release(), conversions[0], CurrentFunction()));
    }
    else
    {
        arguments.Add(mem.release());
    }
    freeStatement = new Cm::BoundTree::BoundFunctionCallStatement(memFreeFun, std::move(arguments));
}

ThrowStatementBinder::ThrowStatementBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_,
    const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_, Cm::BoundTree::BoundFunction* currentFunction_, Binder& binder_) :
    StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), binder(binder_)
{
}

Cm::Ast::TryStatementNode* GetOwnerTry(Cm::Ast::Node* node)
{
    bool skipTry = false;
    Cm::Ast::Node* parent = node->Parent();
    while (parent)
    {
        if (parent->IsCatchNode())
        {
            skipTry = true;
        }
        else if (parent->IsTryStatementNode())
        {
            if (skipTry)
            {
                skipTry = false;
            }
            else
            {
                return static_cast<Cm::Ast::TryStatementNode*>(parent);
            }
        }
        parent = parent->Parent();
    }
    return nullptr;
}

void ThrowStatementBinder::EndVisit(Cm::Ast::ThrowStatementNode& throwStatementNode)
{
    Cm::Ast::TryStatementNode* ownerTry = GetOwnerTry(&throwStatementNode);
    if (ownerTry == nullptr && CurrentFunction()->GetFunctionSymbol()->IsNothrow())
    {
        throw Cm::Core::Exception("nothrow function cannot throw", throwStatementNode.GetSpan(), CurrentFunction()->GetFunctionSymbol()->GetSpan());
    }
    std::unique_ptr<Cm::BoundTree::BoundExpression> exceptionExpr(Pop());
    Cm::Sym::TypeSymbol* exceptionType = exceptionExpr->GetType();
    Cm::Sym::TypeSymbol* plainExceptionType = SymbolTable().GetTypeRepository().MakePlainType(exceptionType);
    if (plainExceptionType->IsClassTypeSymbol())
    {
        Cm::Sym::ClassTypeSymbol* exceptionClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(plainExceptionType);
        if (exceptionClassType->IsAbstract())
        {
            throw Cm::Core::Exception("cannot instantiate an abstract class", throwStatementNode.GetSpan());
        }
        Cm::Sym::Symbol* systemExceptionSymbol = SymbolTable().GlobalScope()->Lookup("System.Exception");
        if (systemExceptionSymbol)
        {
            if (systemExceptionSymbol->IsClassTypeSymbol())
            {
                Cm::Sym::ClassTypeSymbol* systemExceptionClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(systemExceptionSymbol);
                if (Cm::Sym::TypesEqual(exceptionClassType, systemExceptionClassType) || exceptionClassType->HasBaseClass(systemExceptionClassType))
                {
                    Cm::Sym::ExceptionTable* exceptionTable = Cm::Sym::GetExceptionTable();
                    exceptionTable->AddProjectException(exceptionClassType);
                    int32_t exceptionId = exceptionTable->GetExceptionId(exceptionClassType);
                    Cm::Parser::FileRegistry* fileRegistry = Cm::Parser::GetCurrentFileRegistry();
                    std::string sourceFilePath = fileRegistry->GetParsedFileName(throwStatementNode.GetSpan().FileIndex());
                    int32_t sourceLineNumber = throwStatementNode.GetSpan().LineNumber();
                    Cm::Ast::NewNode* newEx = new Cm::Ast::NewNode(throwStatementNode.GetSpan(), new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exceptionClassType->FullName()));
                    Cm::Ast::CloneContext cloneContext;
                    newEx->AddArgument(throwStatementNode.ExceptionExpr()->Clone(cloneContext));
                    std::string exVarName = CurrentFunction()->GetNextTempVariableName();
                    Cm::Ast::DerivationList pointerDerivation;
                    pointerDerivation.Add(Cm::Ast::Derivation::pointer);
                    Cm::Ast::DerivedTypeExprNode* exPtrType = new Cm::Ast::DerivedTypeExprNode(throwStatementNode.GetSpan(), pointerDerivation, 
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exceptionClassType->FullName()));
                    Cm::Ast::ConstructionStatementNode* constructEx = new Cm::Ast::ConstructionStatementNode(throwStatementNode.GetSpan(), exPtrType, 
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exVarName));
                    constructEx->AddArgument(newEx);
                    Cm::Ast::InvokeNode* setFileExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::ArrowNode(throwStatementNode.GetSpan(), new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exVarName),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "SetFile")));
                    Cm::Ast::StringLiteralNode* sourceFilePathArg = new Cm::Ast::StringLiteralNode(throwStatementNode.GetSpan(), sourceFilePath);
                    setFileExpr->AddArgument(sourceFilePathArg);
                    Cm::Ast::SimpleStatementNode* setFileStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), setFileExpr);
                    Cm::Ast::InvokeNode* setLineExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::ArrowNode(throwStatementNode.GetSpan(), new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exVarName),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "SetLine")));
                    setLineExpr->AddArgument(new Cm::Ast::IntLiteralNode(throwStatementNode.GetSpan(), sourceLineNumber));
                    Cm::Ast::SimpleStatementNode* setLineStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), setLineExpr);

                    Cm::Ast::InvokeNode* beginCaptureCallStackCall = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "begin_capture_call_stack"));
                    Cm::Ast::SimpleStatementNode* beginCaptureCallStackStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), beginCaptureCallStackCall);
                    Cm::Ast::InvokeNode* captureCallStackCall = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(), 
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "capture_call_stack"));
                    Cm::Ast::InvokeNode* setCallStackExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::ArrowNode(throwStatementNode.GetSpan(), new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exVarName),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "SetCallStack")));
                    setCallStackExpr->AddArgument(captureCallStackCall);
                    Cm::Ast::SimpleStatementNode* setCallStackStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), setCallStackExpr);
                    Cm::Ast::InvokeNode* endCaptureCallStackCall = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "end_capture_call_stack"));
                    Cm::Ast::SimpleStatementNode* endCaptureCallStackStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), endCaptureCallStackCall);

                    Cm::Ast::InvokeNode* callThisThreadExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(), 
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "this_thread"));
                    Cm::Ast::InvokeNode* callGetExceptionTableAddrExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "get_exception_table_addr"));
                    callGetExceptionTableAddrExpr->AddArgument(callThisThreadExpr);
                    Cm::Ast::InvokeNode* callSetExceptionAddrExpr = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), "System.Support.SetExceptionAddr"));
                    callSetExceptionAddrExpr->AddArgument(callGetExceptionTableAddrExpr);
                    Cm::Ast::IntLiteralNode* exceptionIdArg = new Cm::Ast::IntLiteralNode(throwStatementNode.GetSpan(), exceptionId);
                    callSetExceptionAddrExpr->AddArgument(exceptionIdArg);
                    callSetExceptionAddrExpr->AddArgument(new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), exVarName));
                    Cm::Ast::SimpleStatementNode* setExceptionAddrStatement = new Cm::Ast::SimpleStatementNode(throwStatementNode.GetSpan(), callSetExceptionAddrExpr);

                    Cm::Ast::AssignmentStatementNode* setExceptionCodeStatement = new Cm::Ast::AssignmentStatementNode(throwStatementNode.GetSpan(),
                        new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), Cm::IrIntf::GetExCodeVarName()),
                        new Cm::Ast::IntLiteralNode(throwStatementNode.GetSpan(), exceptionId));
                    Cm::Ast::AssignmentStatementNode* setExceptionCodeParamStatement = nullptr;
                    Cm::Ast::StatementNode* endOfThrowStatement = nullptr;
                    Cm::Ast::FunctionNode* functionNode = throwStatementNode.GetFunction();
                    Cm::Ast::TryStatementNode* ownerTry = GetOwnerTry(&throwStatementNode);
                    if (ownerTry)
                    {
                        int catchId = CurrentFunction()->GetNextCatchId();
                        ownerTry->SetFirstCatchId(catchId);
                        endOfThrowStatement = new Cm::Ast::GotoStatementNode(throwStatementNode.GetSpan(), new Cm::Ast::LabelNode(throwStatementNode.GetSpan(), "$C" + std::to_string(catchId)));
                    }
                    else
                    {
                        setExceptionCodeParamStatement = new Cm::Ast::AssignmentStatementNode(throwStatementNode.GetSpan(),
                            new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), Cm::IrIntf::GetExceptionCodeParamName()),
                            new Cm::Ast::IdentifierNode(throwStatementNode.GetSpan(), Cm::IrIntf::GetExCodeVarName()));
                        Cm::Ast::Node* returnTypeExpr = functionNode->ReturnTypeExpr();
                        if (returnTypeExpr && returnTypeExpr->GetNodeType() != Cm::Ast::NodeType::voidNode)
                        {
                            Cm::Ast::CloneContext cloneContext;
                            Cm::Ast::InvokeNode* defaultValue = new Cm::Ast::InvokeNode(throwStatementNode.GetSpan(), returnTypeExpr->Clone(cloneContext));
                            endOfThrowStatement = new Cm::Ast::ReturnStatementNode(throwStatementNode.GetSpan(), defaultValue);
                        }
                        else
                        {
                            endOfThrowStatement = new Cm::Ast::ReturnStatementNode(throwStatementNode.GetSpan(), nullptr);
                        }
                    }

                    Cm::Ast::CompoundStatementNode* throwActions = new Cm::Ast::CompoundStatementNode(throwStatementNode.GetSpan());
                    CurrentFunction()->Own(throwActions);
                    throwActions->SetLabelNode(throwStatementNode.Label());
                    throwActions->SetParent(functionNode);
                    throwActions->AddStatement(constructEx);
                    throwActions->AddStatement(setFileStatement);
                    throwActions->AddStatement(setLineStatement);
                    throwActions->AddStatement(beginCaptureCallStackStatement);
                    throwActions->AddStatement(setCallStackStatement);
                    throwActions->AddStatement(endCaptureCallStackStatement);
                    throwActions->AddStatement(setExceptionAddrStatement);
                    throwActions->AddStatement(setExceptionCodeStatement);
                    if (setExceptionCodeParamStatement)
                    {
                        throwActions->AddStatement(setExceptionCodeParamStatement);
                    }
                    throwActions->AddStatement(endOfThrowStatement);
                    Cm::Sym::DeclarationVisitor declarationVisitor(SymbolTable());
                    throwActions->Accept(declarationVisitor);
                    Cm::Sym::ContainerScope* containerScope = SymbolTable().GetContainerScope(throwActions);
                    containerScope->SetParent(ContainerScope());
                    binder.BeginContainerScope(containerScope);
                    throwActions->Accept(binder);
                    binder.EndContainerScope();
                }
                else
                {
                    throw Cm::Core::Exception("exception must be class type equal to or derived from System.Exception class", throwStatementNode.GetSpan());
                }
            }
            else
            {
                throw std::runtime_error("System.Exception symbol not class type");
            }
        }
        else
        {
            throw std::runtime_error("System.Exception symbol not found");
        }
    }
    else
    {
        throw Cm::Core::Exception("type of exception must be class type", throwStatementNode.GetSpan());
    }
}

TryBinder::TryBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Binder& binder_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), binder(binder_)
{
}

void TryBinder::Visit(Cm::Ast::TryStatementNode& tryStatementNode)
{
    Cm::Ast::CatchNode* firstHandler = tryStatementNode.GetFirstHandler();
    if (firstHandler->CatchId() == -1)
    {
        firstHandler->SetCatchId(CurrentFunction()->GetNextCatchId());
    }
    binder.AddBoundStatement(new Cm::BoundTree::BoundBeginTryStatement(&tryStatementNode, firstHandler->CatchId()));
    int continueId = CurrentFunction()->GetNextCatchId();
    std::string continueLabel = "$C" + std::to_string(continueId);
    tryStatementNode.TryBlock()->Accept(binder);
    binder.AddBoundStatement(new Cm::BoundTree::BoundEndTryStatement(&tryStatementNode));
    Cm::Ast::GotoStatementNode* gotoOverCatches = new Cm::Ast::GotoStatementNode(tryStatementNode.GetSpan(), new Cm::Ast::LabelNode(tryStatementNode.GetSpan(), continueLabel));
    CurrentFunction()->Own(gotoOverCatches);
    gotoOverCatches->Accept(binder);
    tryStatementNode.Handlers().Accept(binder);
    Cm::Ast::SimpleStatementNode* emptyStatement = new Cm::Ast::SimpleStatementNode(tryStatementNode.GetSpan(), nullptr);
    CurrentFunction()->Own(emptyStatement);
    emptyStatement->SetLabelNode(new Cm::Ast::LabelNode(tryStatementNode.GetSpan(), continueLabel));
    emptyStatement->Accept(binder);
}

CatchBinder::CatchBinder(Cm::BoundTree::BoundCompileUnit& boundCompileUnit_, Cm::Sym::ContainerScope* containerScope_, const std::vector<std::unique_ptr<Cm::Sym::FileScope>>& fileScopes_,
    Cm::BoundTree::BoundFunction* currentFunction_, Binder& binder_) : StatementBinder(boundCompileUnit_, containerScope_, fileScopes_, currentFunction_), binder(binder_)
{
}

void CatchBinder::Visit(Cm::Ast::CatchNode& catchNode)
{
    if (catchNode.CatchId() == -1)
    {
        catchNode.SetCatchId(CurrentFunction()->GetNextCatchId());
    }
    catchNode.ExceptionTypeExpr()->Accept(*this);
    std::unique_ptr<Cm::BoundTree::BoundExpression> exceptionTypeExpr(Pop());
    if (exceptionTypeExpr->IsBoundTypeExpression())
    {
        Cm::BoundTree::BoundTypeExpression* boundTypeExpression = static_cast<Cm::BoundTree::BoundTypeExpression*>(exceptionTypeExpr.get());
        Cm::Sym::TypeSymbol* catchedExceptionType = boundTypeExpression->Symbol();
        Cm::Sym::TypeSymbol* plainCatchedExceptionType = SymbolTable().GetTypeRepository().MakePlainType(catchedExceptionType);
        if (plainCatchedExceptionType->IsClassTypeSymbol())
        {
            Cm::Sym::ClassTypeSymbol* catchedExceptionClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(plainCatchedExceptionType);
            if (catchedExceptionClassType->IsAbstract())
            {
                throw Cm::Core::Exception("cannot instantiate an abstract class", catchNode.GetSpan());
            }
            Cm::Sym::Symbol* systemExceptionSymbol = SymbolTable().GlobalScope()->Lookup("System.Exception");
            if (systemExceptionSymbol)
            {
                if (systemExceptionSymbol->IsClassTypeSymbol())
                {
                    Cm::Sym::ClassTypeSymbol* systemExceptionClassType = static_cast<Cm::Sym::ClassTypeSymbol*>(systemExceptionSymbol);
                    if (Cm::Sym::TypesEqual(catchedExceptionClassType, systemExceptionClassType) || catchedExceptionClassType->HasBaseClass(systemExceptionClassType))
                    {
                        Cm::Sym::ExceptionTable* exceptionTable = Cm::Sym::GetExceptionTable();
                        exceptionTable->AddProjectException(catchedExceptionClassType);
                        int32_t catchedExceptionId = exceptionTable->GetExceptionId(catchedExceptionClassType);
                        Cm::Ast::IdentifierNode* handleVarId = new Cm::Ast::IdentifierNode(catchNode.GetSpan(), CurrentFunction()->GetNextTempVariableName());
                        Cm::Ast::ConstructionStatementNode* handleThisExStatement = new Cm::Ast::ConstructionStatementNode(catchNode.GetSpan(), new Cm::Ast::BoolNode(catchNode.GetSpan()), handleVarId);
                        handleThisExStatement->SetLabelNode(new Cm::Ast::LabelNode(catchNode.GetSpan(), "$C" + std::to_string(catchNode.CatchId())));
                        Cm::Ast::InvokeNode* handleThisExCall = new Cm::Ast::InvokeNode(catchNode.GetSpan(), new Cm::Ast::IdentifierNode(catchNode.GetSpan(), "System.Support.HandleThisEx"));
                        handleThisExCall->AddArgument(new Cm::Ast::IdentifierNode(catchNode.GetSpan(), Cm::IrIntf::GetExceptionBaseIdTableName()));
                        handleThisExCall->AddArgument(new Cm::Ast::IdentifierNode(catchNode.GetSpan(), Cm::IrIntf::GetExCodeVarName()));
                        Cm::Ast::CloneContext cloneContext;
                        handleThisExCall->AddArgument(new Cm::Ast::IntLiteralNode(catchNode.GetSpan(), catchedExceptionId));
                        handleThisExStatement->AddArgument(handleThisExCall);
                        Cm::Ast::CompoundStatementNode* handlerBlock = new Cm::Ast::CompoundStatementNode(catchNode.GetSpan());
                        CurrentFunction()->Own(handlerBlock);
                        Cm::Ast::FunctionNode* functionNode = catchNode.GetFunction();
                        handlerBlock->SetParent(functionNode);

                        handlerBlock->AddStatement(handleThisExStatement);
                        Cm::Ast::Node* parent = catchNode.Parent();
                        if (!parent->IsTryStatementNode())
                        {
                            throw std::runtime_error("not try node");
                        }
                        Cm::Ast::TryStatementNode* tryStatementNode = static_cast<Cm::Ast::TryStatementNode*>(parent);
                        if (tryStatementNode->IsLastHandler(&catchNode))
                        {
                            Cm::Ast::CompoundStatementNode* propagateExStatement = new Cm::Ast::CompoundStatementNode(catchNode.GetSpan());
                            Cm::Ast::TryStatementNode* ownerTry = GetOwnerTry(tryStatementNode);
                            if (ownerTry)
                            {
                                Cm::Ast::CatchNode* outerHandler = ownerTry->GetFirstHandler();
                                if (outerHandler->CatchId() == -1)
                                {
                                    outerHandler->SetCatchId(CurrentFunction()->GetNextCatchId());
                                }
                                propagateExStatement->AddStatement(new Cm::Ast::GotoStatementNode(catchNode.GetSpan(), 
                                    new Cm::Ast::LabelNode(catchNode.GetSpan(), "$C" + std::to_string(outerHandler->CatchId()))));
                            }
                            else
                            {
                                if (CurrentFunction()->GetFunctionSymbol()->CanThrow())
                                {
                                    Cm::Ast::AssignmentStatementNode* setExceptionCodeParamStatement = new Cm::Ast::AssignmentStatementNode(catchNode.GetSpan(),
                                        new Cm::Ast::IdentifierNode(catchNode.GetSpan(), Cm::IrIntf::GetExceptionCodeParamName()),
                                        new Cm::Ast::IdentifierNode(catchNode.GetSpan(), Cm::IrIntf::GetExCodeVarName()));
                                    propagateExStatement->AddStatement(setExceptionCodeParamStatement);
                                }
                                Cm::Ast::Node* returnTypeExpr = functionNode->ReturnTypeExpr();
                                if (returnTypeExpr && returnTypeExpr->GetNodeType() != Cm::Ast::NodeType::voidNode)
                                {
                                    Cm::Ast::InvokeNode* defaultValue = new Cm::Ast::InvokeNode(catchNode.GetSpan(), returnTypeExpr->Clone(cloneContext));
                                    Cm::Ast::ReturnStatementNode* returnStatement = new Cm::Ast::ReturnStatementNode(catchNode.GetSpan(), defaultValue);
                                    propagateExStatement->AddStatement(returnStatement);
                                }
                                else
                                {
                                    Cm::Ast::ReturnStatementNode* returnStatement = new Cm::Ast::ReturnStatementNode(catchNode.GetSpan(), nullptr);
                                    propagateExStatement->AddStatement(returnStatement);
                                }
                            }
                            Cm::Ast::ConditionalStatementNode* dontHandleTest = new Cm::Ast::ConditionalStatementNode(catchNode.GetSpan(),
                                new Cm::Ast::NotNode(catchNode.GetSpan(), handleVarId->Clone(cloneContext)), propagateExStatement, nullptr);
                            handlerBlock->AddStatement(dontHandleTest);
                        }
                        else
                        {
                            Cm::Ast::CatchNode* nextHandler = tryStatementNode->GetNextHandler(&catchNode);
                            int nextCatchId = nextHandler->CatchId();
                            if (nextHandler->CatchId() == -1)
                            {
                                nextCatchId = CurrentFunction()->GetNextCatchId();
                                nextHandler->SetCatchId(nextCatchId);
                            }
                            Cm::Ast::GotoStatementNode* gotoNextHandler = new Cm::Ast::GotoStatementNode(catchNode.GetSpan(), new Cm::Ast::LabelNode(catchNode.GetSpan(), "$C" + std::to_string(nextCatchId)));
                            Cm::Ast::ConditionalStatementNode* dontHandleTest = new Cm::Ast::ConditionalStatementNode(catchNode.GetSpan(),
                                new Cm::Ast::NotNode(catchNode.GetSpan(), handleVarId->Clone(cloneContext)), gotoNextHandler, nullptr);
                            handlerBlock->AddStatement(dontHandleTest);
                        }
                        Cm::Ast::InvokeNode* callThisThreadExpr = new Cm::Ast::InvokeNode(catchNode.GetSpan(),
                            new Cm::Ast::IdentifierNode(catchNode.GetSpan(), "this_thread"));
                        Cm::Ast::InvokeNode* callGetExceptionTableAddrExpr = new Cm::Ast::InvokeNode(catchNode.GetSpan(),
                            new Cm::Ast::IdentifierNode(catchNode.GetSpan(), "get_exception_table_addr"));
                        callGetExceptionTableAddrExpr->AddArgument(callThisThreadExpr);
                        Cm::Ast::InvokeNode* callGetExceptionAddrExpr = new Cm::Ast::InvokeNode(catchNode.GetSpan(),
                            new Cm::Ast::IdentifierNode(catchNode.GetSpan(), "System.Support.GetExceptionAddr"));
                        callGetExceptionAddrExpr->AddArgument(callGetExceptionTableAddrExpr);
                        callGetExceptionAddrExpr->AddArgument(new Cm::Ast::IdentifierNode(catchNode.GetSpan(), Cm::IrIntf::GetExCodeVarName()));
                        Cm::Ast::DerivationList pointerDerivation;
                        pointerDerivation.Add(Cm::Ast::Derivation::pointer);
                        Cm::Ast::DerivedTypeExprNode* exPtrType = new Cm::Ast::DerivedTypeExprNode(catchNode.GetSpan(), pointerDerivation, new Cm::Ast::IdentifierNode(catchNode.GetSpan(),
                            catchedExceptionClassType->FullName()));
                        std::string exPtrVarName = CurrentFunction()->GetNextTempVariableName();
                        Cm::Ast::ConstructionStatementNode* constructExPtrStatement = new Cm::Ast::ConstructionStatementNode(catchNode.GetSpan(), exPtrType,
                            new Cm::Ast::IdentifierNode(catchNode.GetSpan(), exPtrVarName));
                        constructExPtrStatement->AddArgument(new Cm::Ast::CastNode(catchNode.GetSpan(), exPtrType->Clone(cloneContext), callGetExceptionAddrExpr));
                        handlerBlock->AddStatement(constructExPtrStatement);
                        Cm::Ast::DerivationList constRefDerivation;
                        constRefDerivation.Add(Cm::Ast::Derivation::const_);
                        constRefDerivation.Add(Cm::Ast::Derivation::reference);
                        Cm::Ast::Node* exTypeExpr = catchNode.ExceptionTypeExpr()->Clone(cloneContext);
                        Cm::Ast::IdentifierNode* exId = nullptr;
                        if (catchNode.ExceptionId())
                        {
                            exId = static_cast<Cm::Ast::IdentifierNode*>(catchNode.ExceptionId()->Clone(cloneContext));
                        }
                        else
                        {
                            exId = new Cm::Ast::IdentifierNode(catchNode.GetSpan(), CurrentFunction()->GetNextTempVariableName());
                        }
                        Cm::Ast::ConstructionStatementNode* constructExVarStatement = new Cm::Ast::ConstructionStatementNode(catchNode.GetSpan(), exTypeExpr, exId);
                        constructExVarStatement->AddArgument(new Cm::Ast::DerefNode(catchNode.GetSpan(), new Cm::Ast::IdentifierNode(catchNode.GetSpan(), exPtrVarName)));
                        handlerBlock->AddStatement(constructExVarStatement);
                        Cm::Ast::TemplateIdNode* exDeleterType = new Cm::Ast::TemplateIdNode(catchNode.GetSpan(), new Cm::Ast::IdentifierNode(catchNode.GetSpan(), "System.Support.ExDeleter"));
                        exDeleterType->AddTemplateArgument(new Cm::Ast::IdentifierNode(catchNode.GetSpan(), catchedExceptionClassType->FullName()));
                        Cm::Ast::ConstructionStatementNode* constructExDeleterStatement = new Cm::Ast::ConstructionStatementNode(catchNode.GetSpan(), exDeleterType,
                            new Cm::Ast::IdentifierNode(catchNode.GetSpan(), CurrentFunction()->GetNextTempVariableName()));
                        constructExDeleterStatement->AddArgument(new Cm::Ast::IdentifierNode(catchNode.GetSpan(), exPtrVarName));
                        handlerBlock->AddStatement(constructExDeleterStatement);
                        handlerBlock->AddStatement(static_cast<Cm::Ast::StatementNode*>(catchNode.CatchBlock()->Clone(cloneContext)));
                        Cm::Sym::DeclarationVisitor declarationVisitor(SymbolTable());
                        handlerBlock->Accept(declarationVisitor);
                        Cm::Sym::ContainerScope* containerScope = SymbolTable().GetContainerScope(handlerBlock);
                        containerScope->SetParent(ContainerScope());
                        binder.BeginContainerScope(containerScope);
                        handlerBlock->Accept(binder);
                        binder.EndContainerScope();
                    }
                    else
                    {
                        throw Cm::Core::Exception("exception must be class type equal to or derived from System.Exception class", catchNode.GetSpan());
                    }
                }
                else
                {
                    throw std::runtime_error("System.Exception symbol not class type");
                }
            }
            else
            {
                throw std::runtime_error("System.Exception symbol not found");
            }
        }
        else
        {
            throw Cm::Core::Exception("type of exception must be class type", catchNode.GetSpan());
        }
    }
    else
    {
        throw Cm::Core::Exception("exception type expression has no type", catchNode.GetSpan());
    }
}

} } // namespace Cm::Bind