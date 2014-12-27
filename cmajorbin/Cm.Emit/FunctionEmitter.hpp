/*========================================================================
    Copyright (c) 2012-2015 Seppo Laakko
    http://sourceforge.net/projects/cmajor/

    Distributed under the GNU General Public License, version 3 (GPLv3).
    (See accompanying LICENSE.txt or http://www.gnu.org/licenses/gpl.html)

========================================================================*/

#ifndef CM_EMIT_FUNCTION_EMITTER_INCLUDED
#define CM_EMIT_FUNCTION_EMITTER_INCLUDED
#include <Cm.BoundTree/Visitor.hpp>
#include <Cm.Core/GenData.hpp>
#include <Cm.Core/IrFunctionRepository.hpp>
#include <Cm.Sym/LocalVariableSymbol.hpp>

namespace Cm { namespace Emit {

class LocalVariableIrObjectRepository
{
public:
    Ir::Intf::Object* CreateLocalVariableIrObjectFor(Cm::Sym::LocalVariableSymbol *localVariable);
    Ir::Intf::Object* GetLocalVariableIrObject(Cm::Sym::LocalVariableSymbol *localVariable);
private:
    typedef std::unordered_map<Cm::Sym::LocalVariableSymbol*, Ir::Intf::Object*>  LocalVariableObjectMap;
    typedef LocalVariableObjectMap::const_iterator LocalVariableObjectMapIt;
    LocalVariableObjectMap localVariableObjectMap;
    std::vector<std::unique_ptr<Ir::Intf::Object>> ownedIrObjects;
    std::unordered_set<std::string> assemblyNames;
    std::string MakeUniqueAssemblyName(const std::string& name);
};

class FunctionEmitter : public Cm::BoundTree::Visitor
{
public:
    FunctionEmitter(Cm::Util::CodeFormatter& codeFormatter_, Cm::Sym::TypeRepository& typeRepository_, Cm::Core::IrFunctionRepository& irFunctionRepository_);
    void BeginVisit(Cm::BoundTree::BoundFunction& boundFunction) override;
    void EndVisit(Cm::BoundTree::BoundFunction& boundFunction) override;

    void Visit(Cm::BoundTree::BoundLiteral& boundLiteral) override;
    void Visit(Cm::BoundTree::BoundConstant& boundConstant) override;
    void Visit(Cm::BoundTree::BoundLocalVariable& boundLocalVariable) override;
    void Visit(Cm::BoundTree::BoundMemberVariable& boundMemberVariable) {}
    void Visit(Cm::BoundTree::BoundConversion& boundConversion) override;
    void Visit(Cm::BoundTree::BoundCast& boundCast) {}
    void Visit(Cm::BoundTree::BoundUnaryOp& boundUnaryOp) {}
    void Visit(Cm::BoundTree::BoundBinaryOp& boundBinaryOp) override;
    void Visit(Cm::BoundTree::BoundFunctionCall& functionCall) override;
    void Visit(Cm::BoundTree::BoundDisjunction& boundDisjunction) {}
    void Visit(Cm::BoundTree::BoundConjunction& boundConjunction) {}

    void Visit(Cm::BoundTree::BoundCompoundStatement& boundCompoundStatement) {}
    void Visit(Cm::BoundTree::BoundReturnStatement& boundReturnStatement) {}
    void Visit(Cm::BoundTree::BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(Cm::BoundTree::BoundAssignmentStatement& boundAssignmentStatement) {}
    void Visit(Cm::BoundTree::BoundThrowStatement& boundThrowStatement) {}
    void Visit(Cm::BoundTree::BoundSimpleStatement& boundSimpleStatement) {}
    void Visit(Cm::BoundTree::BoundSwitchStatement& boundSwitchStatement) {}
    void Visit(Cm::BoundTree::BoundBreakStatement& boundBreakStatement) {}
    void Visit(Cm::BoundTree::BoundContinueStatement& boundContinueStatement) {}
    void Visit(Cm::BoundTree::BoundConditionalStatement& boundConditionalStatement) {}
    void Visit(Cm::BoundTree::BoundDoStatement& boundDoStatement) {}
    void Visit(Cm::BoundTree::BoundWhileStatement& boundWhileStatement) {}
    void Visit(Cm::BoundTree::BoundForStatement& boundForStatement) {}
    void Visit(Cm::BoundTree::BoundTryStatement& boundTryStatement) {}
private:
    Cm::Util::CodeFormatter& codeFormatter;
    std::unique_ptr<Cm::Core::Emitter> emitter;
    Cm::Sym::TypeRepository& typeRepository;
    Cm::Core::GenFlags genFlags;
    Cm::Core::GenResultStack resultStack;
    Cm::Core::IrFunctionRepository& irFunctionRepository;
    LocalVariableIrObjectRepository localVariableIrObjectRepository;
    void GenerateCall(Cm::Sym::FunctionSymbol* fun, Cm::Core::GenResult& result);
    void GenJumpingBoolCode(Cm::Core::GenResult& result);
};

} } // namespace Cm::Emit

#endif // CM_EMIT_FUNCTION_EMITTER_INCLUDED
